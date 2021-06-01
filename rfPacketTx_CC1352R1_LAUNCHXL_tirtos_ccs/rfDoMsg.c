/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/
#include "tx_defines.h"

/***** Variables and Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"

void mx_do_msg(void) {
    char msg_buf[MAX_LENGTH - HEADER_LEN];
    uint8_t i = 0;

    while(1) {
           memset(msg_buf, 0, sizeof(msg_buf));
           memset(packet, 0, sizeof(packet));

           for (i = 0; i < sizeof(msg_buf); i++) {
               UART2_read(uart, &msg_buf[i], 1, NULL);
               UART2_write(uart, &msg_buf[i], 1, NULL);

               if (msg_buf[i] == '\r') {
                   UART2_write(uart, "\n", 1, NULL);
                   if (strcmp(msg_buf, "lorem\r") == 0) {
                       memcpy(msg_buf, LOREM_IPSUM, sizeof(msg_buf));
                   }
                   break;
               }
           }


           memset(packet, 0, sizeof(packet));
           packet[PKG_ID_BYTE] = MSG_PKG;

           memcpy(&packet[HEADER_LEN], msg_buf, sizeof(msg_buf));
           terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);

//
           switch(terminationReason) {
                  case RF_EventLastCmdDone:
                      UART2_write(uart, "Sending msg pkg done. SENT:\n\r", sizeof("Sending msg pkg done SENT:\n\r"), NULL);
                      UART2_write(uart, packet, MAX_LENGTH, NULL);
                      UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);

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

           uint32_t cmdStatus = ((volatile RF_Op*)&RF_cmdPropTx)->status;
           switch(cmdStatus)        {
               case PROP_DONE_OK:
                   // Packet transmitted successfully
                   break;
               case PROP_DONE_STOPPED:
                   // received CMD_STOP while transmitting packet and finished
                   // transmitting packet
                   break;
               case PROP_DONE_ABORT:
                   // Received CMD_ABORT while transmitting packet
                   break;
               case PROP_ERROR_PAR:
                   // Observed illegal parameter
                   break;
               case PROP_ERROR_NO_SETUP:
                   // Command sent without setting up the radio in a supported
                   // mode using CMD_PROP_RADIO_SETUP or CMD_RADIO_SETUP
                   break;
               case PROP_ERROR_NO_FS:
                   // Command sent without the synthesizer being programmed
                   break;
               case PROP_ERROR_TXUNF:
                   // TX underflow observed during operation
                   break;
               default:
                   // Uncaught error event - these could come from the
                   // pool of states defined in rf_mailbox.h
                   while(1);
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
