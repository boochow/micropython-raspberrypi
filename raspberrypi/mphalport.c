#include <unistd.h>
#include "py/mpconfig.h"
#include "systick.h"

void mp_hal_delay_ms(mp_uint_t ms) {
    uint64_t end_time;

    end_time = systime() + ms * 1000;
    while(systime() < end_time);
  
    return;
}

void mp_hal_delay_us(mp_uint_t us) {
    uint64_t end_time;

    end_time = systime() + us;
    while(systime() < end_time);
  
    return;
}

mp_uint_t mp_hal_ticks_us(void) {
    return systime() / 1000;
}

mp_uint_t mp_hal_ticks_ms(void) {
    return systime();
}

