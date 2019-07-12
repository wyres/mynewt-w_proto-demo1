/**
 Wyres private code
 */
#include <stdio.h>
#include <stdarg.h>

#include "os/os.h"
#include "bsp.h"
#include "console/console.h"
#include "hal/hal_gpio.h"
#include "wutils.h"

#define DEBUG 1

/**
 * replace system assert with a more gentle one
 */
void wassert_fn(const char* f, const char* l) {
    // log
    // TODO
    // flash led
    hal_gpio_init_out(LED_D1, 1);
    hal_gpio_init_out(LED_D2, 0);
    // if debugging,  halt busy flashing leds
    // else reboot
#ifdef DEBUG
    while(1) {
        int total = 0;
        // busy wait - don't let OS do anything or run another task
        for(int i=0; i<1000000; ++i) {
            // do something to avoid being optimised out
            ++total;
        }
        hal_gpio_toggle(LED_D1);        
        hal_gpio_toggle(LED_D2);
    }
#else /* DEBUG */
    reboot();
#endif /* DEBUG */
}

void log_debug_fn(const char* ls, ...) {
    va_list vl;
    char buf[256];
    va_start(vl, ls);

    vsprintf(buf, ls, vl);
    // send to mynewt logger

    va_end(vl);
}
void log_warn_fn(const char* ls, ...) {
    va_list vl;
    char buf[256];
    va_start(vl, ls);

    vsprintf(buf, ls, vl);
    // send to mynewt logger

    va_end(vl);
}
void log_error_fn(const char* ls, ...) {
    va_list vl;
    char buf[256];
    va_start(vl, ls);

    vsprintf(buf, ls, vl);
    // send to mynewt logger
    console_write(buf, 256);
    va_end(vl);
}