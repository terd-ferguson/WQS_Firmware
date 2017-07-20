#include "wqs.h"

//This is called by HTTP when it needs new values
Void updateValues()
{

        static uint8_t         i;
        static uint8_t         txBuffer[5];
        static uint8_t         rxBuffer[20];

        for(i=0;i<sizeof(rxBuffer);i++)
        {
            rxBuffer[i]=0;
        }

        for(i=0;i<sizeof(txBuffer);i++)
        {
            txBuffer[i]=0;
        }

        I2C_Params_init(&i2cParams);
        i2cParams.bitRate = I2C_100kHz;
        i2cParams.transferMode  = I2C_MODE_BLOCKING; // other tasks continue during I2C process
        EK_TM4C1294XL_initI2C();
        i2c = I2C_open(Board_I2C_TMP, &i2cParams);

        txBuffer[0] = (uint8_t)'R';

        tx(txBuffer, 0x66);
        tx(txBuffer, 0x64);
        tx(txBuffer, 0x63);
        tx(txBuffer, 0x62);
        tx(txBuffer, 0x61);

        //Task_sleep(1000);
        SysCtlDelay(40e6);

        temp = (float)atof((const char *) rx(rxBuffer, 0x66));
        Orp = (float)atof((const char *) rx(rxBuffer, 0x62));
        Ph = (float)atof((const char *) rx(rxBuffer, 0x63));
        Do = (float)atof((const char *) rx(rxBuffer, 0x61));
        elec = (float)atof((const char *) rx(rxBuffer, 0x64));

        /* Deinitialized I2C */
        I2C_close(i2c);

        System_flush();
}

void allign_data(uint8_t *rx)
{
    uint8_t i;
    while(rx[0]<=32)
    {
        for(i=0;i<19;i++)
        {
            rx[i]=rx[i+1];
        }
        rx[19]=0;
    }
}

uint8_t getLength(uint8_t *cmd)
{
    uint8_t i=0;
    // find how many characters long the command is
    while(cmd[i]!='\0')
    {
        i++;
    }
    // return the command length
    return i;
}

void tx(uint8_t *cmd, uint8_t addr)
{
    i2cTransaction.slaveAddress = addr;
    i2cTransaction.writeBuf = cmd;
    i2cTransaction.writeCount = getLength(cmd);
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;
    I2C_transfer(i2c, &i2cTransaction);
}

uint8_t *rx(uint8_t *buff, uint8_t addr)
{
    i2cTransaction.slaveAddress = addr;
    i2cTransaction.writeBuf = NULL;
    i2cTransaction.writeCount = 0;
    i2cTransaction.readBuf = buff;
    i2cTransaction.readCount = 20;
    I2C_transfer(i2c, &i2cTransaction);
    allign_data(buff);
    return buff;
}
