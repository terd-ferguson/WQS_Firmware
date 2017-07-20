#ifndef I2C_H_
#define I2C_H_

#include <xdc/std.h>
#include <ti/drivers/I2C.h>

#include <driverlib/sysctl.h>
#include <stdlib.h>

void tx(uint8_t *, uint8_t);
uint8_t *rx(uint8_t *, uint8_t);
uint8_t getLength(uint8_t *);
void allign_data(uint8_t *);
//void function(void);

I2C_Handle      i2c;
I2C_Params      i2cParams;
I2C_Transaction i2cTransaction;

#endif /* I2C_H_ */
