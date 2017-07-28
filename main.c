#include "wqs.h"

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initEMAC();

    /*Check Debug Input Status*/
    checkDebug();

    /* Start Status LED */
    led_status = LED_BLINK;
    startLed();

    /* Start BIOS */
    BIOS_start();

    return (0);
}



