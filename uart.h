#ifndef UART_H_
#define UART_H_

#include <xdc/std.h>
#include <ti/drivers/UART.h>

#include <driverlib/sysctl.h>
#include <stdlib.h>

// uart funcions //
void uart0_config(void);
void uart3_config(void);
void R(uint8_t); // EDIT THIS
void selector(uint8_t);
void wrr(uint8_t, uint8_t *, uint8_t);
void save_to_float(uint8_t cs);

UART_Handle     uart0;
UART_Params     uartParams0;
UART_Handle     uart3;
UART_Params     uartParams3;

static uint8_t         rxBuffer[40];

//uint8_t                crnl[2]={0x0D, 0x0A};
//uint8_t                cr[1]={0x0D};

#endif /* UART_H_ */

//
///* XDCtools Header files */
//#include <xdc/std.h>
//#include <xdc/runtime/System.h>
//
///* BIOS Header files */
//#include <ti/sysbios/BIOS.h>
//#include <ti/sysbios/knl/Task.h>
//
///* TI-RTOS Header files */
//#include <ti/drivers/GPIO.h>
//#include <ti/drivers/UART.h>
//
///* Example/Board Header files */
//#include "Board.h"
//
//#include <inc/hw_memmap.h>
//#include <stdbool.h>
//#include <stdlib.h>
//#include <stdint.h>
//#include <driverlib/sysctl.h>
//#include "uart.h"
