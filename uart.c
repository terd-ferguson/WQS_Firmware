#include "wqs.h"

uint8_t                crnl[2]={0x0D, 0x0A};
uint8_t                cr[1]={0x0D};

//This is called by HTTP when it needs new values
Void updateValues()
{
    uint8_t i;

    // take readings
    for(i=1;i<=5;i++)
    {
        R(i);
    }
    // close uart

    // flush sys
    System_flush();
}

void uart3_config(void)
{
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams3);
    uartParams3.readEcho = UART_ECHO_OFF;
    uartParams3.baudRate = 115200;
    uart3 = UART_open(Board_UART3, &uartParams3);
    System_printf("\r\nuart 3 has been enabled.\r\n");
    System_flush();
}

void uart0_config(void)
{
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams0);
    uartParams0.readEcho = UART_ECHO_OFF;
    uartParams0.baudRate = 115200;
    uart0 = UART_open(Board_UART0, &uartParams0);
    System_printf("\r\nuart 0 has been enabled.\r\n");
    System_flush();
}

void selector(uint8_t x)
{
    x--;
    GPIO_write(PE1, x&4);
    GPIO_write(PE2, x&2);
    GPIO_write(PE3, x&1);
}

void wrr(uint8_t cs, uint8_t *cmd, uint8_t size)
{
    static uint8_t j;
    for(j=0;j<sizeof(rxBuffer);j++)
    {
        rxBuffer[j]=0;
    }
    selector(cs);
    UART_write(uart3, cmd, size);
    UART_read(uart3, rxBuffer, sizeof(rxBuffer));
    save_to_float(cs);
    UART_write(uart0, rxBuffer, sizeof(rxBuffer));
    UART_write(uart0, cr, sizeof(cr));
    for(j=0;j<sizeof(rxBuffer);j++)
    {
        rxBuffer[j]=0;
    }
    UART_read(uart3, rxBuffer, sizeof(rxBuffer));
    UART_write(uart0, rxBuffer, sizeof(rxBuffer));
    UART_write(uart0, crnl, sizeof(crnl));
    for(j=0;j<sizeof(rxBuffer);j++)
    {
        rxBuffer[j]=0;
    }
}

void R(uint8_t cs)
{
    uint8_t cmd[]={'R', 0x0D};
    wrr(cs, cmd, sizeof(cmd));
}

void save_to_float(uint8_t cs)
{
    switch(cs){
    case 1:
        temp = (float)atof((const char *) rxBuffer);
        break;
    case 2:
        Orp = (float)atof((const char *) rxBuffer);
        break;
    case 3:
        elec = (float)atof((const char *) rxBuffer);
        break;
    case 4:
        Do = (float)atof((const char *) rxBuffer);
        break;
    case 5:
        Ph = (float)atof((const char *) rxBuffer);
        break;
    }
}
