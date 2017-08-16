#include "wqs.h"

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

Void calibrationMode()
{
    calibration();

    activeUpdateValues = &updateValues;

    System_flush();
}

static uint8_t rxBuffer[40];
//static uint8_t temp[40];
uint8_t crnl[2] = { 0x0D, 0x0A };
uint8_t cr[1] = { 0x0D };
uint8_t check1[9] = { 'C', 'h', 'e', 'c', 'k', ' ', '1', 0x0D, 0x0A };
uint8_t test[20];

uint8_t ER[] = { '*', 'E', 'R' };
uint8_t UV[] = { '*', 'U', 'V' };
uint8_t OV[] = { '*', 'O', 'V' };
uint8_t OK[] = { '*', 'O', 'K' };

//float temp=0, Orp=0, elec=0, Do=0, Ph=0;

//struct error_flags {
//    volatile sig_atomic_t timeout;
//    volatile sig_atomic_t er;
//    volatile sig_atomic_t ov;
//    volatile sig_atomic_t uv;
//    volatile sig_atomic_t ok;
//    volatile sig_atomic_t baudChange;
//    volatile sig_atomic_t on;
//    volatile sig_atomic_t factory_reset;
//    volatile sig_atomic_t timeout_CF;
//    volatile sig_atomic_t uv_CF;
//    volatile sig_atomic_t ov_CF;
//    volatile sig_atomic_t er_CF;
//}
struct error_flags
RTD={0,0,0,0,0,0,1,0,0,0,0,0},
ORP={0,0,0,0,0,0,1,0,0,0,0,0},
EC={0,0,0,0,0,0,1,0,0,0,0,0},
DO={0,0,0,0,0,0,1,0,0,0,0,0},
pH={0,0,0,0,0,0,1,0,0,0,0,0};

int status = 1;
bool er = 0;
bool ov = 0;
bool uv = 0;
bool ok = 0;
volatile bool timeout=0, timeout_error=0;
uint8_t active_devices = 0x1F; // initialize devices 1-5 to on state: 0001 1111
int peek = 0;

void uart3_config(uint16_t timeout, uint32_t baud)
{
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams3);
    uartParams3.readTimeout = timeout;
    uartParams3.readEcho = UART_ECHO_OFF;
    uartParams3.baudRate = baud;
    uart3 = UART_open(Board_UART3, &uartParams3);
}

void uart0_config(void)
{
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams0);
    //uartParams0.readTimeout = 30000;
    uartParams0.readEcho = UART_ECHO_OFF;
    uartParams0.baudRate = 115200;
    uart0 = UART_open(Board_UART0, &uartParams0);
    System_printf("\r\nuart 0 has been enabled.\r\n");
    System_flush();
}

void sel(uint8_t x)
{
    x--;
    GPIO_write(PE1, x&4);
    GPIO_write(PE2, x&2);
    GPIO_write(PE3, x&1);
}

void write(uint8_t cs, uint8_t * cmd, uint8_t size)
{
    if(is_on(cs))
    {
        sel(cs);
        UART_write(uart3, cmd, size);
    }
}

/* read function. to be called after a cmd is sent or just to clear uart buffers.
 * this function reads data while it is available. if no data is read upon first
 * read, the function times out in 1000 ms. if data is received, the timeout is
 * reduced to 100 ms and each buffer is crossreferenced with fault detection and
 * error codes.
 */
void read(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t num_reads = 0; // # of buffer reads until timeout
        sel(cs); // select current device
        while (!timeout) // while we are still receiving data...
        {
            clear_buffer(); // clean buffer to examine after read

            if(num_reads>0) // subsequent reads
            {
                UART_control(uart3, UART_CMD_ISAVAILABLE, &peek); // see if there are available bytes to read in circular buffer
                                                                  // if there are, store a '1' in peek, else a '0'
                if(peek==1) // if bytes are available to read...
                {
                    //UART_write(uart0, "\r\navailable = 1\r\n", 17);
                    status=UART_read(uart3, rxBuffer, sizeof(rxBuffer)); // read uart data and store in rxBuffer. return # of bytes received.
                }
                else // if the ring buffer is empty...
                {
                    timeout=1; // on a subsequent read, if there are no bytes to read, issue a timeout
                    //UART_write(uart0, "\r\navailable = 0\r\n", 17);
                }
            }
            else // for first read
                status = UART_read(uart3, rxBuffer, sizeof(rxBuffer)); // read uart data and store in rxBuffer. return # of bytes received.
                                                                       // this should be the first read
            if (status == 0)
            {
                timeout = 1; // if no bytes were received, UART_read has timed out. data has stopped being sent.
                if(num_reads==0)
                    timeout_error=1;
            }
            else
            {
                if(num_reads==0)
                {
                    save_to_float(cs); // save rxBuffer to corresponding device float variable for reading
                }
                UART_write(uart0, rxBuffer, sizeof(rxBuffer)); // print buffer to putty
                UART_write(uart0, cr, sizeof(cr)); // carriage return(0xD)
                buffer_check(cs, rxBuffer); // cross reference buffer with fault detection codes
                num_reads++; // how many times actual data has been read
            }
            assert(cs, num_reads); // assert all faults
        }
        timeout = 0; // reset timeout
    }
}

void buffer_read(void)
{
    static uint8_t j;
    for (j=0;j<sizeof(rxBuffer);j++)
    {
        rxBuffer[j] = 0;
    }
    UART_read(uart0, rxBuffer, sizeof(rxBuffer));
    UART_write(uart0, rxBuffer, sizeof(rxBuffer));
    //UART_write(uart0, crnl, sizeof(crnl));
}

void write_status(void)
{
    System_printf("\r\nStatus = %i\r\n", status);
    System_flush();
}

void buffer_check(uint8_t cs, uint8_t *buff)
{
    if(is_on(cs))
    {
        if (buff[0] == '*')
        {
            if (buff[1] == 'E' && buff[2] == 'R')
            {
                increment_er(cs);
                UART_write(uart0, "Valid *ER Response\r\n", 20);
            }
            else if (buff[1] == 'O')
            {
                if (buff[2] == 'V')
                {
                    increment_ov(cs);
                    UART_write(uart0, "Valid *OV Response\r\n", 20);
                }
                else if (buff[2] == 'K')
                {
                    increment_ok(cs); // increment ok count
                    UART_write(uart0, "Valid *OK Response\r\n", 20);
                }
            }
            else if (buff[1] == 'U' && buff[2] == 'V')
            {
                increment_uv(cs);
                UART_write(uart0, "Valid *UV Response\r\n", 20);
            }
        }
        else if(buff[0]=='?')
        {
            if(buff[1]=='C')
            {
                if(buff[3]=='0')
                {
                    blinky=0;
                }
            }
        }
    }
}

// this function asserts the fault detection
void assert(uint8_t cs, uint8_t num_reads)
{
    if(is_on(cs))
    {
        if(read_ovs(cs)>=2)
        {
            clear_ov(cs);
            turn_off(cs);
            set_ov_cf(cs);
        }
        if(read_uvs(cs)>=2)
        {
            clear_uv(cs);
            turn_off(cs);
            set_uv_cf(cs);
        }
        if(read_ers(cs)>=2 && read_FR(cs)==0)
        {
            clear_er(cs);
            Factory(cs);
        }
        else if(read_ers(cs)>=2 && read_FR(cs)>0)
        {
            turn_off(cs);
            set_er_cf(cs);
        }
        if(timeout_error==1)
        {
         assert_timeout(cs);
         timeout_error=0;
        }
        if(read_oks(cs)>=2)
            clear_errors(cs);
    }
}

/* this function uses a bitwise AND and accumulator to realize the number
 * of devices currently on. the number of devices currently on is stored
 * as a binary value in 8-bit value 'active_devices'. devices available are
 * denoted with an 'x': 000x xxxx, with LSB as device 1 and MSB as device 5
 */
uint8_t num_devices(void)
{
    static uint8_t j;
    uint8_t sum=0;

    for(j=1;j<=5;j++)
    {
        sum+=j&active_devices; // add up the number of active devices
        j=j<<1; // shift left for next operation
    }
    return sum; // return number of active devices
}

/* returns a 1 if the selected device is on, 0 otherwise */
bool is_on(uint8_t cs)
{
    uint8_t j=1;
    return (j<<(cs-1))&active_devices;
}

void turn_off(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t j=1;
        active_devices=(~(j<<(cs-1)))&active_devices; // turn off selected device. ex: (1111 1011)&(0001 1111)=(0001 1011) device 3 is turned off.
        device_status(cs);
    }
}

void assert_timeout(uint8_t cs)
{
    if(is_on(cs))
    {
        switch(cs){
        case 1:
            if(RTD.timeout>=2 && RTD.baudChange==0)
            {
                RTD.timeout=0;
                assert_baud(cs);
            }
            else if(RTD.timeout>=2 && RTD.baudChange==1)
            {
                turn_off(cs);
                set_timeout_cf(cs);
            }
            else
                RTD.timeout++;
            break;
        case 2:
            if(ORP.timeout>=2 && ORP.baudChange==0)
            {
                ORP.timeout=0;
                assert_baud(cs);
            }
            else if(ORP.timeout>=2 && ORP.baudChange==1)
            {
                turn_off(cs);
                set_timeout_cf(cs);
            }
            else
                ORP.timeout++;
            break;
        case 3:
            if(EC.timeout>=2 && EC.baudChange==0)
            {
                EC.timeout=0;
                assert_baud(cs);
            }
            else if(EC.timeout>=2 && EC.baudChange==1)
            {
                turn_off(cs);
                set_timeout_cf(cs);
            }
            else
                EC.timeout++;
            break;
        case 4:
            if(DO.timeout>=2 && DO.baudChange==0)
            {
                DO.timeout=0;
                assert_baud(cs);
            }
            else if(DO.timeout>=2 && DO.baudChange==1)
            {
                turn_off(cs);
                set_timeout_cf(cs);
            }
            else
                DO.timeout++;
            break;
        case 5:
            if(pH.timeout>=2 && pH.baudChange==0)
            {
                pH.timeout=0;
                assert_baud(cs);
            }
            else if(pH.timeout>=2 && pH.baudChange==1)
            {
                turn_off(cs);
                set_timeout_cf(cs);
            }
            else
                pH.timeout++;
            break;
        }
    }
}

void assert_baud(uint8_t cs)
{
    if(is_on(cs))
    {
        blinky=1;
        baud_change(cs);
        UART_close(uart3);
        uart3_config(3000, 9600);
        do{
            if(!is_on(cs))
            {
                break;
            }
            C_0(cs);
            C_Q(cs);
        }while(blinky==1);
        blinky=0;
        decrement_er(cs);
        Baud_n(cs, 115200);
        read(cs);
        UART_close(uart3);
        uart3_config(1000, 115200);
    }
    clear_errors(cs);
}

void change_timeout(uint16_t timeout)
{
    UART_close(uart3);
    uart3_config(timeout, 115200);
}

void clear_buffer(void)
{
    static uint8_t j, size = sizeof(rxBuffer);

    for (j=0;j<size;j++)
    {
        rxBuffer[j] = 0;
    }
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

void R(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'R', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void i(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'i', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void Find(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'F', 'i', 'n', 'd', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }

}

void Status(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'S', 't', 'a', 't',  'u', 's', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void L_1(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'L', ',', '1', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }

}
void L_0(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'L', ',', '0', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void L_Q(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'L', ',', '?', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void Plock_1(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'P', 'l', 'o', 'c', 'k', ',', '1', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void Plock_0(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'P', 'l', 'o', 'c', 'k', ',', '0', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void Plock_Q(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'P', 'l', 'o', 'c', 'k', ',', '?', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void Sleep(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'S', 'l', 'e', 'e', 'p', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void Wake(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void Baud_n(uint8_t cs, uint32_t baud)
{
    if(is_on(cs))
    {
        uint8_t cmd1[]={'B', 'a', 'u', 'd', ',', '9', '6', '0', '0', 0x0D};
        uint8_t cmd2[]={'B', 'a', 'u', 'd', ',', '1', '1', '5', '2', '0', '0', 0x0D};

        if(baud==9600)
        {
            write(cs, cmd1, sizeof(cmd1));
        }
        else if(baud==115200)
        {
            write(cs, cmd2, sizeof(cmd2));
        }
        read(cs);
    }
}
void Baud_Q(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'B', 'a', 'u', 'd', ',', '?', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void Factory(uint8_t cs)
{
    if(is_on(cs))
    {
        change_timeout(3000);
        blinky=1;
        uint8_t cmd[]={'F', 'a', 'c', 't', 'o', 'r', 'y', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
        do{
            if(!is_on(cs))
            {
                break;
            }
            C_0(cs);
            C_Q(cs);
        }while(blinky==1);
        change_timeout(1000);
        increment_FR(cs);
        clear_errors(cs);
    }
}

void C_1(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', ',', '1', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void C_0(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', ',', '0', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void C_n(uint8_t cs, uint8_t rate);
void C_Q(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', ',', '?', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

void Cal_clear(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', 'a', 'l', ',', 'c', 'l', 'e', 'a', 'r', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void Cal_Q(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', 'a', 'l', ',', '?', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
void Cal_t(uint8_t cs) // RTD
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', 'a', 'l', ',', '1', '0', '0', '.', '0', '0', 0x0D};
        write(1, cmd, sizeof(cmd));
        read(1);
    }
}
void Cal(uint8_t cs) // DO, calibrate to atmospheric O2 levels
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', 'a', 'l', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}
/*void Cal_0(uint8_t cs) // DO, not going to use this...
{
    uint8_t cmd[]={'C', 'a', 'l', ',', '0', 0x0D};
    write(cs, cmd, sizeof(cmd));
    read(cs);
}*/
void Cal_dry(uint8_t cs) // EC
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', 'a', 'l', ',', 'd', 'r', 'y', 0x0D};
        write(3, cmd, sizeof(cmd));
        read(3);
    }
}
void Cal_low_n(uint8_t cs) // EC == 3, pH == 5
{
    if(is_on(cs))
    {
        uint8_t cmd1[]={'C', 'a', 'l', ',', 'l', 'o', 'w', ',', '4', '.', '0', 0x0D};
        uint8_t cmd2[]={'C', 'a', 'l', ',', 'l', 'o', 'w', ',', '1', '2', '8', '8', '0', 0x0D};

        if(cs==5)
        {
            write(cs, cmd1, sizeof(cmd1));
            read(cs);
        }
        else if(cs==3)
        {
            write(cs, cmd2, sizeof(cmd2));
            read(cs);
        }
    }
}
void Cal_high_n(uint8_t cs) // EC == 3, pH == 5
{
    if(is_on(cs))
    {
        uint8_t cmd1[]={'C', 'a', 'l', ',', 'h', 'i', 'g', 'h', ',', '1', '0', '.', '0', 0x0D};
        uint8_t cmd2[]={'C', 'a', 'l', ',', 'h', 'i', 'g', 'h', ',', '1', '5', '0', '0', '0', '0', 0x0D};

        if(cs==5)
        {
            write(cs, cmd1, sizeof(cmd1));
            read(cs);
        }
        else if(cs==3)
        {
            write(cs, cmd2, sizeof(cmd2));
            read(cs);
        }
    }
}
void Cal_mid_n(uint8_t cs) // pH
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', 'a', 'l', ',', 'm', 'i', 'd', ',', '7', '.', '0', 0x0D};
        write(5, cmd, sizeof(cmd));
        read(5);
    }
}
void Cal_n(uint8_t cs) // not EC, ORP...only use for ORP!
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'C', 'a', 'l', ',', '2', '2', '5', 0x0D};
        write(2, cmd, sizeof(cmd));
        read(2);
    }
}

void K_n(uint8_t cs) // EC only?
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'K', ',', '1', '.', '0', 0x0D};
        write(3, cmd, sizeof(cmd));
        read(3);
    }
}
void K_Q(uint8_t cs) // EC only?
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'K', ',', '?', 0x0D};
        write(3, cmd, sizeof(cmd));
        read(3);
    }
}

void S_c(uint8_t cs) // RTD specific
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'S', ',', 'c', 0x0D};
        write(1, cmd, sizeof(cmd));
        read(1);
    }
}
void S_k(uint8_t cs) // RTD specific
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'S', ',', 'k', 0x0D};
        write(1, cmd, sizeof(cmd));
        read(1);
    }
}
void S_f(uint8_t cs) // RTD specific
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'S', ',', 'f', 0x0D};
        write(1, cmd, sizeof(cmd));
        read(1);
    }
}

void bad_cmd(uint8_t cs)
{
    if(is_on(cs))
    {
        uint8_t cmd[]={'z', 0x0D};
        write(cs, cmd, sizeof(cmd));
        read(cs);
    }
}

// calibration functions //
void RTD_calibration(void)
{
    write_status();
    UART_write(uart0, "\r\nRTD: Celcius mode\r\n", 21);
    S_c(1); // put in Celcius
    SysCtlDelay(40e6);
    UART_write(uart0, "Calibrate temp", 14);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_t(1); // calibrate
    SysCtlDelay(40e6);
    Cal_Q(1); // query calibration
    SysCtlDelay(40e6);
}
void ORP_calibration(void)
{
    UART_write(uart0, "\r\nCalibrate ORP", 16);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_n(1); // calibrate
    SysCtlDelay(40e6);
    Cal_Q(1); // query calibration
    SysCtlDelay(40e6);
}
void EC_calibration(void)
{
    UART_write(uart0, "\r\nCalibrate EC", 14);
    UART_write(uart0, "\r\nSet probe type\r\n", 18);
    K_n(3);
    SysCtlDelay(40e6);
    K_Q(3);
    SysCtlDelay(40e6);
    UART_write(uart0, "\r\nDry calibration", 17);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_dry(3);
    SysCtlDelay(40e6);
    UART_write(uart0, "\r\nLow calibration", 17);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_low_n(3);
    SysCtlDelay(40e6);
    UART_write(uart0, "\r\nHigh calibration", 18);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_high_n(3);
    SysCtlDelay(40e6);
    Cal_Q(3);
    SysCtlDelay(40e6);
}

/* simple DO calibration function. press "Enter for calibration measurement"
 *
 */
void DO_calibration(void)
{
    UART_write(uart0, "\r\nCalibrate DO", 15);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal(4); // calibrate
    SysCtlDelay(40e6);
    Cal_Q(4); // query calibration
    SysCtlDelay(40e6);
}

/* simple pH calibration function. prompts the user to calibrate for mid, low, and
 * high calibration. press "Enter" for calibratoin measurement.
 */
void pH_calibration(void)
{
    UART_write(uart0, "\r\nCalibrating pH", 16);
    UART_write(uart0, "\r\nMid calibration", 17);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_mid_n(5);
    SysCtlDelay(40e6);
    UART_write(uart0, "\r\nLow calibration", 17);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_low_n(5);
    SysCtlDelay(40e6);
    UART_write(uart0, "\r\nHigh calibration", 18);
    UART_write(uart0, "\r\nPress Enter to take calibration measurement\r\n", 47);
    buffer_read();
    Cal_high_n(5);
    SysCtlDelay(40e6);
    Cal_Q(5);
    SysCtlDelay(40e6);
}

/* main calibration function. cycles through all of the individual calibration
 * functions once.
 *
 * this function should be updated to check pre-existing calibration
 */
void calibration(void)
{
    RTD_calibration();
    ORP_calibration();
    EC_calibration();
    DO_calibration();
    pH_calibration();
}

void decrement_er(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.er--;
        break;
    case 2:
        ORP.er--;
        break;
    case 3:
        EC.er--;
        break;
    case 4:
        DO.er--;
        break;
    case 5:
        pH.er--;
        break;
    }
}

void increment_er(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.er++;
        break;
    case 2:
        ORP.er++;
        break;
    case 3:
        EC.er++;
        break;
    case 4:
        DO.er++;
        break;
    case 5:
        pH.er++;
        break;
    }
}

void increment_ov(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.ov++;
        break;
    case 2:
        ORP.ov++;
        break;
    case 3:
        EC.ov++;
        break;
    case 4:
        DO.ov++;
        break;
    case 5:
        pH.ov++;
        break;
    }
}

void increment_uv(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.uv++;
        break;
    case 2:
        ORP.uv++;
        break;
    case 3:
        EC.uv++;
        break;
    case 4:
        DO.uv++;
        break;
    case 5:
        pH.uv++;
        break;
    }
}

void increment_ok(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.ok++;
        break;
    case 2:
        ORP.ok++;
        break;
    case 3:
        EC.ok++;
        break;
    case 4:
        DO.ok++;
        break;
    case 5:
        pH.ok++;
        break;
    }
}

void clear_timeouts(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.timeout=0;
        break;
    case 2:
        ORP.timeout=0;
        break;
    case 3:
        EC.timeout=0;
        break;
    case 4:
        DO.timeout=0;
        break;
    case 5:
        pH.timeout=0;
        break;
    }
}

void clear_er(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.er=0;
        break;
    case 2:
        ORP.er=0;
        break;
    case 3:
        EC.er=0;
        break;
    case 4:
        DO.er=0;
        break;
    case 5:
        pH.er=0;
        break;
    }
}

void clear_ov(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.ov=0;
        break;
    case 2:
        ORP.ov=0;
        break;
    case 3:
        EC.ov=0;
        break;
    case 4:
        DO.ov=0;
        break;
    case 5:
        pH.ov=0;
        break;
    }
}

void clear_uv(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.uv=0;
        break;
    case 2:
        ORP.uv=0;
        break;
    case 3:
        EC.uv=0;
        break;
    case 4:
        DO.uv=0;
        break;
    case 5:
        pH.uv=0;
        break;
    }
}

void clear_ok(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.ok=0;
        break;
    case 2:
        ORP.ok=0;
        break;
    case 3:
        EC.ok=0;
        break;
    case 4:
        DO.ok=0;
        break;
    case 5:
        pH.ok=0;
        break;
    }
}

void baud_change(uint8_t cs)
{
    if(is_on(cs))
    {

    }
    switch(cs){
    case 1:
        RTD.baudChange=1;
        break;
    case 2:
        ORP.baudChange=1;
        break;
    case 3:
        EC.baudChange=1;
        break;
    case 4:
        DO.baudChange=1;
        break;
    case 5:
        pH.baudChange=1;
        break;
    }
}

void device_status(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.on=0;
        break;
    case 2:
        ORP.on=0;
        break;
    case 3:
        EC.on=0;
        break;
    case 4:
        DO.on=0;
        break;
    case 5:
        pH.on=0;
        break;
    }
}

//bool baud_check(uint8_t cs)
//{
//    bool temp;
//    switch(cs){
//    case 1:
//        if(RTD.baudChange==1)
//            temp=1;
//        else
//            temp=0;
//        break;
//    case 2:
//        if(ORP.baudChange==1)
//            temp=1;
//        else
//            temp=0;
//        break;
//    case 3:
//        if(EC.baudChange==1)
//            temp=1;
//        else
//            temp=0;
//        break;
//    case 4:
//        if(DO.baudChange==1)
//            temp=1;
//        else
//            temp=0;
//        break;
//    case 5:
//        if(pH.baudChange==1)
//            temp=1;
//        else
//            temp=0;
//        break;
//    }
//
//    return temp;
//}

void clear_errors(uint8_t cs)
{
    clear_er(cs);
    clear_ov(cs);
    clear_uv(cs);
    clear_timeouts(cs);
    clear_ok(cs);
}

void increment_FR(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.factory_reset=1;
        break;
    case 2:
        ORP.factory_reset=1;
        break;
    case 3:
        EC.factory_reset=1;
        break;
    case 4:
        DO.factory_reset=1;
        break;
    case 5:
        pH.factory_reset=1;
        break;
    }
}

int read_oks(uint8_t cs)
{
    int oks;
    switch(cs){
    case 1:
        oks=RTD.ok;
        break;
    case 2:
        oks=ORP.ok;
        break;
    case 3:
        oks=EC.ok;
        break;
    case 4:
        oks=DO.ok;
        break;
    case 5:
        oks=pH.ok;
        break;
    }
    return oks;
}

int read_uvs(uint8_t cs)
{
    int uvs;
    switch(cs){
    case 1:
        uvs=RTD.uv;
        break;
    case 2:
        uvs=ORP.uv;
        break;
    case 3:
        uvs=EC.uv;
        break;
    case 4:
        uvs=DO.uv;
        break;
    case 5:
        uvs=pH.uv;
        break;
    }
    return uvs;
}

int read_ovs(uint8_t cs)
{
    int ovs;
    switch(cs){
    case 1:
        ovs=RTD.ov;
        break;
    case 2:
        ovs=ORP.ov;
        break;
    case 3:
        ovs=EC.ov;
        break;
    case 4:
        ovs=DO.ov;
        break;
    case 5:
        ovs=pH.ov;
        break;
    }
    return ovs;
}

int read_ers(uint8_t cs)
{
    int ers;
    switch(cs){
    case 1:
        ers=RTD.er;
        break;
    case 2:
        ers=ORP.er;
        break;
    case 3:
        ers=EC.er;
        break;
    case 4:
        ers=DO.er;
        break;
    case 5:
        ers=pH.er;
        break;
    }
    return ers;
}

int read_FR(uint8_t cs)
{
    int FR;
    switch(cs){
    case 1:
        FR=RTD.factory_reset;
        break;
    case 2:
        FR=ORP.factory_reset;
        break;
    case 3:
        FR=EC.factory_reset;
        break;
    case 4:
        FR=DO.factory_reset;
        break;
    case 5:
        FR=pH.factory_reset;
        break;
    }
    return FR;
}

void set_timeout_cf(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.timeout_CF=1;
        break;
    case 2:
        ORP.timeout_CF=1;
        break;
    case 3:
        EC.timeout_CF=1;
        break;
    case 4:
        DO.timeout_CF=1;
        break;
    case 5:
        pH.timeout_CF=1;
        break;
    }
}

void set_ov_cf(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.ov_CF=1;
        break;
    case 2:
        ORP.ov_CF=1;
        break;
    case 3:
        EC.ov_CF=1;
        break;
    case 4:
        DO.ov_CF=1;
        break;
    case 5:
        pH.ov_CF=1;
        break;
    }
}

void set_uv_cf(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.uv_CF=1;
        break;
    case 2:
        ORP.uv_CF=1;
        break;
    case 3:
        EC.uv_CF=1;
        break;
    case 4:
        DO.uv_CF=1;
        break;
    case 5:
        pH.uv_CF=1;
        break;
    }
}

void set_er_cf(uint8_t cs)
{
    switch(cs){
    case 1:
        RTD.er_CF=1;
        break;
    case 2:
        ORP.er_CF=1;
        break;
    case 3:
        EC.er_CF=1;
        break;
    case 4:
        DO.er_CF=1;
        break;
    case 5:
        pH.er_CF=1;
        break;
    }
}
