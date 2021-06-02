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
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/
#include "tx_defines.h"

/***** Variables and Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"

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
//============================================================================
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
        RF_cmdPropRx.pktConf.bRepeatOk = 0;
        RF_cmdPropRx.pktConf.bRepeatNok = 1;

//============================================================================
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

//    ------------ -- KEYS STUFF -- ---------------
    mx_do_my_keys();
    mx_do_peer_keys();
//    -------------- ------------ -----------------

//    accept user input, split into msgs and send
    mx_do_msg();

    return NULL;
}
//    -------------- ------------ -----------------
