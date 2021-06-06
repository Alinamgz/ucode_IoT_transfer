
/***** Includes *****/
/* Standard C Libraries */
#include "rx_includes.h"

/***** Defines *****/
#include "rx_defines.h"

/***** Prototypes *****/
#include "rx_glob_vars.h"

/***** Function definitions *****/
#include "rx_functions.h"

//  prototypes
static void mx_send_key(void);
static void mx_create_publick_key_pkg(uint8_t *key_pkg, CryptoKey *private_key, CryptoKey *public_key);

void mx_share_my_pub_key(void) {

	mx_create_publick_key_pkg(packet, &private_key, &public_key);
    mx_send_key();
//    UART2_write(uart, "\n\r NU SHO \n\r", sizeof("\n\r NU SHO \n\r"), NULL);
}

void mx_create_publick_key_pkg(uint8_t *key_pkg, CryptoKey *private_key, CryptoKey *public_key) {
    int_fast16_t rslt;

    ECDSA_Handle handle_ecdsa;
    ECDSA_OperationSign operation_sign;

    uint8_t hashed_data_buf[PRIVATE_KEY_LEN];

    uint8_t chck[] = LOREM_IPSUM;

    char check_msg[5];
    int i;


//load pkg metadata n' publick key to pkg
    memset(key_pkg, 0, sizeof(key_pkg));
    key_pkg[PKG_ID_BYTE] = KEY_PKG;
    memcpy(&key_pkg[HEADER_LEN], public_key->u.plaintext.keyMaterial, public_key->u.plaintext.keyLength);

    //================================== Check hash ========================================================
        do_sha(chck, HEADER_LEN + PUBLIC_KEY_LEN, hashed_data_buf, 1);

        UART2_write(uart, "CHECK HASH\n", sizeof("CHECK HASH\n"), NULL);
        for (i = 0; i < SHA2_DIGEST_LENGTH_BYTES_256; i++) {
            memset(check_msg, 0, sizeof(check_msg));
            sprintf(check_msg, " %d", hashed_data_buf[i]);
            UART2_write(uart, check_msg, sizeof(check_msg), NULL);
        }
        UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
    //===================================            =======================================================

/* Perform SHA-2 computation on the data to be signed */


        memset(hashed_data_buf, 0, sizeof(hashed_data_buf));
    do_sha(key_pkg, HEADER_LEN + PUBLIC_KEY_LEN, hashed_data_buf, 1);

UART2_write(uart, "PUB KEY GEN hash\n", sizeof("PUB KEY GEN hash\n"), NULL);
    for (i = 0; i < SHA2_DIGEST_LENGTH_BYTES_256; i++) {
        memset(check_msg, 0, sizeof(check_msg));
        sprintf(check_msg, " %d", hashed_data_buf[i]);
        UART2_write(uart, check_msg, sizeof(check_msg), NULL);
    }

    UART2_write(uart, "\n\r", sizeof("\n\r"), NULL);

    /* Sign some key data to verify to the receiver that this unit has the corresponding private key to the transmit public key pair */

    ECDSA_OperationSign_init(&operation_sign);
    operation_sign.curve = &ECCParams_NISTP256;
    operation_sign.myPrivateKey = private_key;
    operation_sign.hash= hashed_data_buf;
    operation_sign.r = &key_pkg[HEADER_LEN + PUBLIC_KEY_LEN];
    operation_sign.s = &key_pkg[HEADER_LEN + PUBLIC_KEY_LEN + PRIVATE_KEY_LEN];

    handle_ecdsa = ECDSA_open(CONFIG_ECDSA_0, NULL);
    if (!handle_ecdsa) {
        mx_say_err("ECDSA_open()");
    }

    rslt = ECDSA_sign(handle_ecdsa, &operation_sign);
    if (rslt != ECDSA_STATUS_SUCCESS) {
        mx_say_err("ECDSA_sign");
    }

    ECDSA_close(handle_ecdsa);
}

//===============================================================================================================

void mx_send_key(void) {
    terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
    int i = 0;
    char status[4];

    switch(terminationReason) {
        case RF_EventLastCmdDone:
    UART2_write(uart, "Sending key pkg done. SENT:\n\r", sizeof("Sending key pkg done SENT:\n\r"), NULL);
    UART2_write(uart, packet, sizeof(packet), NULL);
    UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
            for (i = 0; i < MAX_LENGTH; i++) {
                memset(status, 0, sizeof(status));
                sprintf(status, " %d", RF_cmdPropTx.pPkt[i]);
                UART2_write(uart, status, sizeof(status), NULL);
            }

            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
            GPIO_toggle(CONFIG_GPIO_LED_RED);

            sleep(1);

            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            break;
        case RF_EventCmdCancelled:
    UART2_write(uart, "Sending key pkg ERR RF_EventCmdCancelled\n\r", sizeof("Sending key pkg ERR RF_EventCmdCancelled\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Command cancelled before it was started; it can be caused
        // by RF_cancelCmd() or RF_flushCmd().
            break;
        case RF_EventCmdAborted:
    UART2_write(uart, "Sending key pkg ERR RF_EventCmdAborted\n\r", sizeof("Sending key pkg ERR RF_EventCmdAborted\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Abrupt command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        case RF_EventCmdStopped:
    UART2_write(uart, "Sending key pkg ERR RF_EventCmdStopped\n\r", sizeof("Sending key pkg ERR RF_EventCmdStopped\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Graceful command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        default:
    UART2_write(uart, "Sending key pkg Uncaught ERR\n\r", sizeof("Sending key pkg Uncaught ERR\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Uncaught error event
            while(1);
    }
}

