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

/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* Board Header files */
#include "ti_drivers_config.h"
#include <ti_radio_config.h>
