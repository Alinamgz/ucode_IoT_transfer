/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/
#include "tx_defines.h"

/***** Variables and Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"
static void mx_read_input(char *msg_buf, ssize_t buf_size, uint8_t *pkg_parts);
static void mx_create_encrypted_pkg(char *msg_buf, uint8_t *packet, uint8_t *nonce, CryptoKey *symmetric_key);
static inline void increment_nonce(uint8_t *nonce, uint8_t size);
static inline bool is_lorem_ipsum(char *msg_buf, ssize_t buf_size, uint8_t *pkg_parts);


void mx_do_msg(void) {
//    NB! msg_buf size MUST be multiple of MSG_LEN
    char msg_buf[4 * MSG_LEN];
    static uint8_t nonce[NONCE_LEN] = {0};

    uint8_t pkg_parts = 1;
    uint8_t i = 0;


    while(1) {
        pkg_parts = 0;
        memset(msg_buf, 0, sizeof(msg_buf));
        mx_read_input(msg_buf, sizeof(msg_buf) - 1, &pkg_parts);

        for (i = 0; i < pkg_parts; i++) {
            /* Packet Header contains packet type, current pkg number and total number of pkgs (ie pkg 1 of 3) */
            memset(packet, 0, sizeof(packet));
            packet[PKG_ID_BYTE] = MSG_PKG;
            packet[CUR_PKG_NUM_BYTE] = 1 + i;
            packet[TOTAL_PKG_NUM_BYTE] = pkg_parts;

            mx_create_encrypted_pkg(&msg_buf[i * MSG_LEN], packet, nonce, &symmetric_key);

            terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
            mx_chck_rf_termination_n_status(terminationReason, ((volatile RF_Op*)&RF_cmdPropTx)->status);
            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
        }

   #ifndef POWER_MEASUREMENT
   //        PIN_setOutputValue(ledPinHandle, CONFIG_PIN_GLED,!PIN_getOutputValue(CONFIG_PIN_GLED));
   #endif
           /* Power down the radio */
           RF_yield(rfHandle);

   #ifdef POWER_MEASUREMENT
           /* Sleep for PACKET_INTERVAL s */
           sleep(PACKET_INTERVAL);
   #else
           /* Sleep for PACKET_INTERVAL us */
           usleep(PACKET_INTERVAL);
   #endif

    }
}

//=========================================================================================
//=========================================================================================

static void mx_read_input(char *msg_buf, ssize_t buf_size, uint8_t *pkg_parts) {
    uint8_t i = 0;
    bool is_lorem = 0;

    for (i = 0; i < buf_size; i++) {
//      reading users input
        UART2_read(uart, &msg_buf[i], 1, NULL);
        UART2_write(uart, &msg_buf[i], 1, NULL);

        if (msg_buf[i] == '\r') {
            is_lorem = is_lorem_ipsum(msg_buf, buf_size, pkg_parts);

            i = is_lorem ? buf_size : i;
            UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
            break;
        }
    }
    *pkg_parts = 1 + i / MSG_LEN;
}

//=========================================================================================
//=========================================================================================
static inline bool is_lorem_ipsum(char *msg_buf, ssize_t buf_size, uint8_t *pkg_parts) {
    if (strcmp(msg_buf, "lorem\r") == 0) {
        memcpy(msg_buf, LOREM_IPSUM, buf_size);

        UART2_write(uart, msg_buf, buf_size, NULL);
        return 1;
    }

    return 0;
}

//=========================================================================================
//=========================================================================================
static void mx_create_encrypted_pkg(char *msg_buf, uint8_t *packet, uint8_t *nonce, CryptoKey *symmetric_key) {
    AESCCM_Handle handle_aesccm;
    AESCCM_Operation operation_aesccm;
    int_fast16_t rslt;


    handle_aesccm = AESCCM_open(CONFIG_AESCCM_0, NULL);
    if (!handle_aesccm) {
        mx_say_err("AESCCM_open");
    }

    increment_nonce(nonce, NONCE_LEN);
    memcpy(&packet[MSG_HEADER_LEN], nonce, NONCE_LEN);

    /* Initialize encryption transaction */
    AESCCM_Operation_init(&operation_aesccm);
    operation_aesccm.key = symmetric_key;

    operation_aesccm.aad = packet;
    operation_aesccm.aadLength = MSG_HEADER_LEN;

    operation_aesccm.nonce = &packet[MSG_HEADER_LEN];
    operation_aesccm.nonceLength = NONCE_LEN;

    operation_aesccm.input = (uint8_t*)msg_buf;
    operation_aesccm.inputLength = MSG_LEN;

    operation_aesccm.output = &packet[MSG_HEADER_LEN + NONCE_LEN];

    operation_aesccm.mac = &packet[MSG_HEADER_LEN + NONCE_LEN + MSG_LEN];
    operation_aesccm.macLength = MAC_LEN;

    /* Execute the encryption transaction */
    rslt = AESCCM_oneStepEncrypt(handle_aesccm, &operation_aesccm);
    if(rslt != AESCCM_STATUS_SUCCESS) {
        mx_say_err("AESCCM_oneStepEncrypt");
    }

    AESCCM_close(handle_aesccm);
}
//=========================================================================================
//=========================================================================================

static inline void increment_nonce(uint8_t *nonce, uint8_t size) {
    uint8_t i = 0;

    bool wrapped = nonce[0] == 254 || nonce[0] == 255;
    nonce[0] += 2;

    for (i = 1; wrapped && nonce[i] == 255 && i < size - 1; ++i) {
        ++nonce[i];
        ++nonce[i + 1];
    }
}
