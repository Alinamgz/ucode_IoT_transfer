/*
 * Copyright (c) 2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPL ARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***** Includes *****/
#include "rx_includes.h"

/***** Defines *****/
#include "rx_defines.h"

/***** Variable declarations *****/
#include "rx_glob_vars.h"

/***** Variable declarations *****/
#include "rx_functions.h"

/***** Prototypes *****/
static void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/***** Function definitions *****/

static inline void mx_init_drivers(void) {
    AESCCM_init();
    ECDH_init();
    ECDSA_init();
    GPIO_init();
    SHA2_init();
    TRNG_init();
}

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

// ========================== UART_2 ===============================
static inline void mx_config_UART2(void) {
    UART2_Params_init(&uart_params);
    uart_params.baudRate = 115200;
//    uart_params.writeMode = UART2_Mode_NONBLOCKING;

    uart = UART2_open(CONFIG_UART2_0, &uart_params);
    if (uart) {
        UART2_write(uart, WELCOME_MSG, sizeof(WELCOME_MSG), NULL);
    }
    else {
        while (1) {
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            usleep(250000);
        }

    }
}

// ==========================   RF   ===============================
static inline void mx_config_RF(void) {
    RF_Params rfParams;

    RF_Params_init(&rfParams);

    RF_cmdPropTx.pktLen = KEY_PKG_LEN;
    RF_cmdPropTx.pPkt = packet;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW;


    if (RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES)) {
        /* Failed to allocate space for all data entries */
        mx_say_err("RFQueue_defineQueue");
    }

    /* Modify CMD_PROP_RX command for application needs */
    /* Set the Data Entity queue for received data */
    RF_cmdPropRx.pQueue = &dataQueue;
    /* Discard ignored packets from Rx queue */
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
    /* Discard packets with CRC error from Rx queue */
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
    /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
    RF_cmdPropRx.maxPktLen = KEY_PKG_LEN;
//    RF_cmdPropRx.pktConf.bRepeatOk = 1;
    RF_cmdPropRx.pktConf.bRepeatOk = 0;
    RF_cmdPropRx.pktConf.bRepeatNok = 1;

    /* Request access to the radio */
#if defined(DeviceFamily_CC26X0R2)
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
#else
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
#endif// DeviceFamily_CC26X0R2

    if (!rfHandle) {
        mx_say_err("RF_open");
    }
    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

}

void *mainThread(void *arg0) {

    mx_init_drivers();
    mx_config_UART2();

    mx_config_RF();

    mx_do_keys();


//    RF_Params rfParams;
////    RF_Params_init(&rfParams);
//
//    if (RFQueue_defineQueue(&dataQueue,
//                            rxDataEntryBuffer,
//                            sizeof(rxDataEntryBuffer),
//                            NUM_DATA_ENTRIES,
//                            MAX_LENGTH + NUM_APPENDED_BYTES)) {
//        /* Failed to allocate space for all data entries */
//        while(1);
//    }

//    /* Modify CMD_PROP_RX command for application needs */
//    /* Set the Data Entity queue for received data */
//    RF_cmdPropRx.pQueue = &dataQueue;
//    /* Discard ignored packets from Rx queue */
//    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
//    /* Discard packets with CRC error from Rx queue */
//    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
//    /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
//    RF_cmdPropRx.maxPktLen = MAX_LENGTH;
//    RF_cmdPropRx.pktConf.bRepeatOk = 1;
//    RF_cmdPropRx.pktConf.bRepeatNok = 1;
//
//    /* Request access to the radio */
//#if defined(DeviceFamily_CC26X0R2)
//    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
//#else
//    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
//#endif// DeviceFamily_CC26X0R2
//
//    /* Set the frequency */
//    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

    /* Enter RX mode and stay forever in RX */

    while (1) {
        terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                               RF_PriorityNormal, &callback,
                                               RF_EventRxEntryDone);

    switch(terminationReason) {
        case RF_EventLastCmdDone:
            UART2_write(uart, "\n\rOK\n\r", 6, NULL);
            if (packet[0] == KEY_PKG) {
                mx_handle_keypkg(packet, &peer_pub_key);
            }
            // A stand-alone radio operation command or the last radio
            // operation command in a chain finished.
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
            UART2_write(uart, "\n\rdefault\n\r", sizeof("\n\rdefault\n\r"), NULL);
            // Uncaught error event
            while(1);
    }
////
    uint32_t cmdStatus = ((volatile RF_Op*)&RF_cmdPropRx)->status;
    switch(cmdStatus) {
        case PROP_DONE_OK:
            UART2_write(uart, "\n\rok\n\r", 6, NULL);
            // Packet received with CRC OK
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
        default:
            UART2_write(uart, "\n\r status default\n\r", sizeof("\n\r status default\n\r"), NULL);
            // Uncaught error event - these could come from the
            // pool of states defined in rf_mailbox.h
            while(1);
    }

        UART2_write(uart, "\n\r--------------------------\n\r", sizeof("\n\r--------------------------\n\r"), NULL);
    }

//    while(1);
}

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e) {

//    char status[64];
//    int i = 0;
//    int_fast16_t rslt;

    if (e & RF_EventRxEntryDone) {

            /* Toggle pin to indicate RX */
            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
    //        GPIO_toggle(CONFIG_GPIO_LED_RED);

            /* Get current unhandled data entry */
            currentDataEntry = RFQueue_getDataEntry();

            /* Handle the packet data, located at &currentDataEntry->data:
             * - Length is the first byte with the current configuration
             * - Data starts from the second byte */
            packetLength      = *(uint8_t*)(&currentDataEntry->data);
            packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

            /* Copy the payload + the status byte to the packet variable */
            memset(packet,0, sizeof(packet));
            memcpy(packet, packetDataPointer, (packetLength + 1));

            switch(*packetDataPointer) {
                case KEY_PKG:
                    UART2_write(uart, "\r\nrecv key:\n\r", sizeof("\r\nrecv key:\n\r"), NULL);
                    GPIO_toggle(CONFIG_GPIO_LED_RED);
                    break;
                case MSG_PKG:
                    UART2_write(uart, "\r\nrecv msg:\n\r", sizeof("\r\nrecv msg:\n\r"), NULL);
                    break;
                default:
                    UART2_write(uart, "!!! ERR: unknown pkg type !!!\\n\r", sizeof("!!! ERR: unknown pkg type !!!\\n\r"), NULL);
                    break;
            }

        UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);

        RFQueue_nextEntry();

    }
}
