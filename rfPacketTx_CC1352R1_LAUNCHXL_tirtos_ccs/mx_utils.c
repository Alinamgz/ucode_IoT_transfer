/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/
#include "tx_defines.h"

/***** Variables and Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"

// ===========================================================================
// ============================== SAY ERR ====================================
// ===========================================================================
inline void mx_say_err(char *where_err) {
    char err_msg[64];

    memset(err_msg, 0, 64);
    sprintf(err_msg, "\n\r!!! %s ERR !!!\n\r", where_err);

    UART2_write(uart, err_msg, sizeof(err_msg), NULL);
    while (1) {
        GPIO_toggle(CONFIG_GPIO_LED_RED);
        usleep(250000);
    }
}

// ===========================================================================
// ============================== SHA-2 ======================================
// ===========================================================================
inline void do_sha (uint8_t* src, uint32_t src_len, uint8_t *rslt_buf, uint8_t chck) {
    SHA2_Handle handle_sha2;
    int_fast16_t rslt;

    /* Hash the sharedSecret to a 256-bit buffer */
    handle_sha2 = SHA2_open(CONFIG_SHA2_0, NULL);
    if (!handle_sha2) {
        mx_say_err("SHA2_open @ aes_key");
    }

    /* As the Y-coordinate is derived from the X-coordinate, hashing only the X component (i.e. keyLength/2 bytes)
     * is a relatively common way of deriving a symmetric key from a shared secret if you are not using a dedicated key derivation function. */

    rslt = SHA2_hashData(handle_sha2, src, src_len, rslt_buf);

    if (rslt != SHA2_STATUS_SUCCESS) {
        mx_say_err("SHA2_hashData @ aes_key");
    }

    SHA2_close(handle_sha2);
}
// ===========================================================================
// ============================== WRITE PKG ==================================
// ===========================================================================

void mx_check_write_pkg(uint8_t *arr, uint32_t arr_len, uint8_t *prompt, uint32_t prompt_len) {
    int i = 0;
    char check_msg[4];

    UART2_write(uart, prompt, prompt_len, NULL);

    for (i = 0; i < arr_len; i++) {
        memset(check_msg, 0, sizeof(check_msg));
        sprintf(check_msg, " %d", arr[i]);
        UART2_write(uart, check_msg, sizeof(check_msg), NULL);
    }

    UART2_write(uart, "\n\r----- ----- -----\n\r", sizeof("\n\r----- ----- -----\n\r"), NULL);

}
