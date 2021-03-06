#ifndef WQS_H_
#define WQS_H_

#include <string.h>
#include <stdio.h>
#include <signal.h>

/* XDCtools Header files */
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpcli.h>

/* Board Header file */
#include "Board.h"

/* UART Header file */
#include "uart.h"

#define LED_OFF 0
#define LED_BLINK 1
#define LED_STABLE 2
sig_atomic_t led_status;

float Orp, Ph, Do, temp, elec;

Void updateValues();
Void debugUpdateValues();
Void calibrationMode();

Void(*activeUpdateValues)();

Void startLed();

Void checkDebug();

#endif
