#include "wqs.h"

Void ledTask(UArg arg0, UArg arg1)
{
    unsigned int led;

    while(1)
    {
        if (led_status == LED_OFF)
        {
            led = 0;
        }
        else if (led_status == LED_STABLE)
        {
            led = 1;
        }
        else if (led_status == LED_BLINK)
        {
            led ^= 1;
        }
        else
        {
            GPIO_write(Board_LED0, 0);
            System_printf("led_status unrecognized value.  Exiting.\n");
            System_flush();
            return;
        }

        GPIO_write(Board_LED0, led);
        Task_sleep(500);
    }
}

Void startLed()
{
    static Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

    Task_Params_init(&taskParams);
    taskParams.stackSize = 512;
    taskParams.priority = 2;
    taskHandle = Task_create((Task_FuncPtr)ledTask, &taskParams, &eb);
    if (taskHandle == NULL) {
        System_printf("startLed: Failed to create status LED Task\n");
        System_flush();
    }
}
