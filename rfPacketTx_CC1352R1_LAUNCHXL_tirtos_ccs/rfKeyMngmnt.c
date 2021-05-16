/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/

#include "tx_defines.h"

/***** Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/

/* Generate random bytes in the provided buffer up to size using the TRNG */
void mx_generate_random_bytes(CryptoKey *entropyKey) {
    TRNG_Handle handle;
    int_fast16_t rslt;
    char err_msg[] = "\n !!! TRNG ERR !!!\n";
    int i = 0;

    /*    call driver init funtion */
    UART2_write(uart, "\nama tuta\r\n", sizeof("\nama tuta\n\r"), NULL);
    TRNG_init();

/*    open a TRNG_Handle with the provided buffer */
    handle = TRNG_open(CONFIG_TRNG_0, NULL);
    if (!handle) {
        UART2_write(uart, err_msg, sizeof(err_msg), NULL);
        while(1) {
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            usleep(500000);
        }
    }

    rslt = TRNG_generateEntropy(handle, entropyKey);

/*    err handling */
    if(rslt != TRNG_STATUS_SUCCESS) {
        UART2_write(uart, err_msg, sizeof(err_msg), NULL);
        while(1) {
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            usleep(500000);
        }

    }
    else {
        for(i = 0; i < 10; i++) {
            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
            usleep(500000);
        }
    }

    TRNG_close(handle);
}
