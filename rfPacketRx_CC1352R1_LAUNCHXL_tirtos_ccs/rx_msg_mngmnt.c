/***** Includes *****/
#include "rx_includes.h"

/***** Defines *****/
#include "rx_defines.h"

/***** Variable declarations *****/
#include "rx_glob_vars.h"

/***** Variable declarations *****/
#include "rx_functions.h"

/***** Function definitions *****/
void mx_decrypt_n_print_msg(uint8_t *packet, CryptoKey *symmetric_key) {
    AESCCM_Handle handle_aesccm;
    AESCCM_Operation operation_aesccm;

    int_fast16_t rslt;
    uint8_t msg_buf[MSG_LEN];

    memset(msg_buf, 0, sizeof(msg_buf));

    handle_aesccm = AESCCM_open(CONFIG_AESECB_0, NULL);
    if (!handle_aesccm) {
        mx_say_err("AESCCM_open");
    }

    AESCCM_Operation_init(&operation_aesccm);

    operation_aesccm.key = symmetric_key;

    operation_aesccm.aad = packet;
    operation_aesccm.aadLength = MSG_HEADER_LEN;

    operation_aesccm.nonce = &packet[MSG_HEADER_LEN];
    operation_aesccm.nonceLength = NONCE_LEN;

    operation_aesccm.input = &packet[MSG_HEADER_LEN + NONCE_LEN];
    operation_aesccm.inputLength = MSG_LEN;

    operation_aesccm.output = msg_buf;

    operation_aesccm.mac = &packet[MSG_HEADER_LEN + NONCE_LEN + MSG_LEN];
    operation_aesccm.macLength = MAC_LEN;

    rslt = AESCCM_oneStepDecrypt(handle_aesccm, &operation_aesccm);
    if (rslt != AESCCM_STATUS_SUCCESS) {
        mx_say_err("AESCCM_oneStepDecrypt");
    }

    AESCCM_close(handle_aesccm);

    UART2_write(uart, msg_buf, sizeof(msg_buf), NULL);

    if (packet[CUR_PKG_NUM_BYTE] == packet[TOTAL_PKG_NUM_BYTE]) {
        UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
    }
}
