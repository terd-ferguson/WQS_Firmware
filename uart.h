#ifndef UART_H_
#define UART_H_

#include <xdc/std.h>
#include <ti/drivers/UART.h>

#include <driverlib/sysctl.h>
#include <stdlib.h>
#include <signal.h>

// cmd functions //
// general funcions //
Void taskFxn(UArg, UArg);
void uart0_config(void);
void uart3_config(uint16_t, uint32_t);
void sel(uint8_t);
void write(uint8_t, uint8_t *, uint8_t);
void read(uint8_t);
void buffer_read(void);
void write_status(void);
void buffer_check(uint8_t, uint8_t *);
void clear_buffer(void);
void assert(uint8_t, uint8_t);
uint8_t num_devices(void);
bool is_on(uint8_t);
void turn_off(uint8_t);
void assert_timeout(uint8_t);
void clear_errors(uint8_t);
void assert_baud(uint8_t);
void decrement_er(uint8_t);
void increment_er(uint8_t);
void increment_ov(uint8_t);
void increment_uv(uint8_t);
void increment_ok(uint8_t);
void increment_FR(uint8_t);
int read_oks(uint8_t);
int read_uvs(uint8_t);
int read_ovs(uint8_t);
int read_ers(uint8_t);
int read_FR(uint8_t);
void change_timeout(uint16_t);
void clear_timeouts(uint8_t);
void clear_ok(uint8_t);
void bad_cmd(uint8_t);
void clear_er(uint8_t);
void clear_ov(uint8_t);
void clear_uv(uint8_t);
void clear_errors(uint8_t);
void set_timeout_cf(uint8_t);
void set_ov_cf(uint8_t);
void set_uv_cf(uint8_t);
void set_er_cf(uint8_t);
void save_to_float(uint8_t);

// cmd functions //
// finished
void R(uint8_t); // check
void i(uint8_t); // check
void Find(uint8_t); // check
void Status(uint8_t); // check
void L_1(uint8_t); // check
void L_0(uint8_t); // check
void L_Q(uint8_t); // check
void Plock_1(uint8_t); // check
void Plock_0(uint8_t); // check
void Plock_Q(uint8_t); // check
void C_1(uint8_t); // might need work?
void C_0(uint8_t); // might need work?
void C_Q(uint8_t); // check
void Cal_clear(uint8_t); // check
void Cal_Q(uint8_t); // check
void S_c(uint8_t); // RTD specific // check
void S_k(uint8_t); // RTD specific
void S_f(uint8_t); // RTD specific
void Cal_t(uint8_t); // RTD // check
void Cal_n(uint8_t); // EC, ORP // check
void Cal(uint8_t); // DO // check
void Cal_dry(uint8_t); // EC // check
void K_n(uint8_t); // EC? // check
void K_Q(uint8_t); // EC? // check
void calibration(void); // check
void RTD_calibration(void); // check
void ORP_calibration(void); // check
void EC_calibration(void); // check
void DO_calibration(void); // check
void pH_calibration(void); // check
void Baud_n(uint8_t, uint32_t); // check
void Baud_Q(uint8_t); // check
void baud_change(uint8_t);
void device_status(uint8_t);
bool baud_check(uint8_t);

// functions in progress //
void Cal_0(uint8_t); // DO
// need to check
void Cal_mid_n(uint8_t); // pH
void Cal_low_n(uint8_t); // EC, pH
void Cal_high_n(uint8_t); // EC, pH

void Sleep(uint8_t); // needs work
void Wake(uint8_t); // needs work?
void Factory(uint8_t); // needs work, for sure.
// unfinished
void C_n(uint8_t, uint8_t);

void Slope_Q(void);

void T_n(uint8_t, float); // DO, EC, pH
void T_Q(uint8_t); // DO, EC, pH
void S_n(uint8_t, float);
void S_n_ppt(uint8_t, float, float);
void S_Q(uint8_t); // RTD, DO
void P_n(uint8_t, float);
void P_Q(uint8_t);
void D_n(uint8_t, uint16_t);
void D_0(uint8_t);
void D_Q(uint8_t);
void M(uint8_t);
void M_all(uint8_t);
void M_Q(uint8_t);
void M_clear(uint8_t);
void *OK_1(uint8_t);
void *OK_0(uint8_t);
void *OK_Q(uint8_t);

UART_Handle     uart0;
UART_Params     uartParams0;
UART_Handle     uart3;
UART_Params     uartParams3;

extern struct error_flags {
    volatile sig_atomic_t timeout;
    volatile sig_atomic_t er;
    volatile sig_atomic_t ov;
    volatile sig_atomic_t uv;
    volatile sig_atomic_t ok;
    volatile sig_atomic_t baudChange;
    volatile sig_atomic_t on;
    volatile sig_atomic_t factory_reset;
    volatile sig_atomic_t timeout_CF;
    volatile sig_atomic_t uv_CF;
    volatile sig_atomic_t ov_CF;
    volatile sig_atomic_t er_CF;
} RTD, ORP, EC, DO, pH;

volatile bool blinky; // flag used for turning off continuous read

#endif /* UART_H_ */
