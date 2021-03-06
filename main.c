#include "wqs.h"

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initUART();
    Board_initEMAC();

    /*Check Debug Input Status*/
    checkDebug();

    uart0_config();
    uart3_config(1000, 115200);

    /* Start Status LED */
    led_status = LED_BLINK;
    startLed();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
