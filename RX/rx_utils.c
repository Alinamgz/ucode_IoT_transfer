/***** Includes *****/
/* Standard C Libraries */
#include "rx_includes.h"

/***** Defines *****/
#include "rx_defines.h"

/***** Variables and Prototypes *****/
#include "rx_glob_vars.h"

/***** Function definitions *****/
#include "rx_functions.h"

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
inline void mx_do_sha (uint8_t* src, uint32_t src_len, uint8_t *rslt_buf, uint8_t chck) {
    SHA2_Handle handle_sha2;
    int_fast16_t rslt;

    /* Hash the sharedSecret to a 256-bit buffer */
    handle_sha2 = SHA2_open(CONFIG_SHA2_0, NULL);
    if (!handle_sha2) {
        mx_say_err("SHA2_open @ aes_key");
    }

    rslt = SHA2_hashData(handle_sha2, src, src_len, rslt_buf);

    if (rslt != SHA2_STATUS_SUCCESS) {
        mx_say_err("SHA2_hashData @ aes_key");
    }

    SHA2_close(handle_sha2);
}
// ===========================================================================
// ============================== PRINT PKG ==================================
// ===========================================================================

inline void mx_print_pkg(uint8_t *arr, uint32_t arr_len, uint8_t *prompt, uint32_t prompt_len) {
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
// ===========================================================================
// ================ CHECK RF TERMINATION REASON 'N STATUS ====================
// ===========================================================================
 inline int_fast16_t mx_chck_rf_termination_n_status(RF_EventMask terminationReason, uint32_t cmdStatus) {
    //  init rslt val -1; each OK adds 1 to rslt val (termination reason Done +1, status +1)
    //  => rslt returned < 1 means we have a problen
    int_fast16_t rslt = -1;

    switch(terminationReason) {
        case RF_EventLastCmdDone:
            // operation command in a chain finished.
            rslt++;
            break;
        case RF_EventCmdCancelled:
            UART2_write(uart, "\n\rRF_EventCmdCancelled\n\r", sizeof("\n\rRF_EventCmdCancelled\n\r"), NULL);
            // Command cancelled before it was started; it can be caused
            // by RF_cancelCmd() or RF_flushCmd().
            break;
        case RF_EventCmdAborted:
            UART2_write(uart, "\n\rRF_EventCmdAborted\n\r", sizeof("\n\rRF_EventCmdAborted\n\r"), NULL);
            // Abrupt command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        case RF_EventCmdStopped:
            UART2_write(uart, "\n\rRF_EventCmdStopped\n\r", sizeof("\n\rRF_EventCmdStopped\n\r"), NULL);
            // Graceful command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        default:
            UART2_write(uart, "\n\rUncaught termination error\n\r", sizeof("\n\rUncaught termination error\n\r"), NULL);
            // Uncaught error event
            while(1);
    }

    switch(cmdStatus) {
            case PROP_DONE_OK:
                // Packet received with CRC OK
                rslt++;
                break;
            case PROP_DONE_RXERR:
                UART2_write(uart, "\n\r status PROP_DONE_RXERR\n\r", sizeof("\n\r status PROP_DONE_RXERR\n\r"), NULL);
                // Packet received with CRC error
                break;
            case PROP_DONE_RXTIMEOUT:
                UART2_write(uart, "\n\r status PROP_DONE_RXTIMEOUT\n\r", sizeof("\n\r status PROP_DONE_RXTIMEOUT\n\r"), NULL);
                // Observed end trigger while in sync search
                break;
            case PROP_DONE_BREAK:
                UART2_write(uart, "\n\r status PROP_DONE_BREAK\n\r", sizeof("\n\r status PROP_DONE_BREAK\n\r"), NULL);
                // Observed end trigger while receiving packet when the command is
                // configured with endType set to 1
                break;
            case PROP_DONE_ENDED:
                UART2_write(uart, "\n\r status PROP_DONE_ENDED\n\r", sizeof("\n\r status PROP_DONE_ENDED\n\r"), NULL);
                // Received packet after having observed the end trigger; if the
                // command is configured with endType set to 0, the end trigger
                // will not terminate an ongoing reception
                break;
            case PROP_DONE_STOPPED:
                UART2_write(uart, "\n\r status PROP_DONE_STOPPED\n\r", sizeof("\n\r status PROP_DONE_STOPPED\n\r"), NULL);
                // received CMD_STOP after command started and, if sync found,
                // packet is received
                break;
            case PROP_DONE_ABORT:
                UART2_write(uart, "\n\r status PROP_DONE_ABORT\n\r", sizeof("\n\r status PROP_DONE_ABORT\n\r"), NULL);
                // Received CMD_ABORT after command started
                break;
            case PROP_ERROR_RXBUF:
                UART2_write(uart, "\n\r status PROP_ERROR_RXBUF\n\r", sizeof("\n\r status PROP_ERROR_RXBUF\n\r"), NULL);
                // No RX buffer large enough for the received data available at
                // the start of a packet
                break;
            case PROP_ERROR_RXFULL:
                UART2_write(uart, "\n\r status PROP_ERROR_RXFULL\n\r", sizeof("\n\r status PROP_ERROR_RXFULL\n\r"), NULL);
                // Out of RX buffer space during reception in a partial read
                break;
            case PROP_ERROR_PAR:
                UART2_write(uart, "\n\r status PROP_ERROR_PAR\n\r", sizeof("\n\r status PROP_ERROR_PAR\n\r"), NULL);
                // Observed illegal parameter
                break;
            case PROP_ERROR_NO_SETUP:
                UART2_write(uart, "\n\r status PROP_ERROR_NO_SETUP\n\r", sizeof("\n\r status PROP_ERROR_NO_SETUP\n\r"), NULL);
                // Command sent without setting up the radio in a supported
                // mode using CMD_PROP_RADIO_SETUP or CMD_RADIO_SETUP
                break;
            case PROP_ERROR_NO_FS:
                UART2_write(uart, "\n\r status PROP_ERROR_NO_FS\n\r", sizeof("\n\r status PROP_ERROR_NO_FS\n\r"), NULL);
                // Command sent without the synthesizer being programmed
                break;
            case PROP_ERROR_RXOVF:
                UART2_write(uart, "\n\r status PROP_ERROR_RXOVF\n\r", sizeof("\n\r status PROP_ERROR_RXOVF\n\r"), NULL);
                // RX overflow observed during operation
                break;
            case PROP_ERROR_TXUNF:
                UART2_write(uart, "\n\r status PROP_ERROR_TXUNF \n\r", sizeof("\n\r status PROP_ERROR_TXUNF \n\r"), NULL);
                // TX underflow observed during operation
                break;
            default:
                UART2_write(uart, "\n\rRF Uncaught error status\n\r", sizeof("\n\rRF Uncaught error status\n\r"), NULL);
                // Uncaught error event - these could come from the
                // pool of states defined in rf_mailbox.h
                while(1);
        }

    return rslt;
}
