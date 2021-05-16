#pragma once

/***** Includes *****/
/* Standard C Libraries */
#include <stdlib.h>
#include <unistd.h>
//#include <string.h>

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/UART2.h>

// for key n' encryption
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SHA2.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/AESCCM.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

//for keys
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>



/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* Board Header files */
#include "ti_drivers_config.h"
#include <ti_radio_config.h>
