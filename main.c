#include "wqs.h"

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initUART();
    Board_initEMAC();

    uart0_config();
    uart3_config();

    /* Start Status LED */
    led_status = LED_BLINK;
    startLed();

    /* Start BIOS */
    BIOS_start();

    return (0);
}



