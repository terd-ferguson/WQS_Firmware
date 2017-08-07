#include "wqs.h"

Void debugUpdateValues()
{
    static float value;

    if (value > 1)
    {
        value = 0;
    }

    value += .1;

    temp = value;
    Orp = value;
    Ph = value;
    Do = value;
    elec = value;

    Task_sleep(1000);
}

Void checkDebug()
{
    unsigned int sw = GPIO_read(Board_BUTTON0);

    //Pressed is apparently 0 for the on board buttons
    if (sw == 0)
    {
        activeUpdateValues = &debugUpdateValues;
    }
    else
    {
        activeUpdateValues = &updateValues;
    }
}
