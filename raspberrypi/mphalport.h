#include "py/obj.h"

void mp_hal_delay_ms(mp_uint_t ms);
void mp_hal_delay_us(mp_uint_t us);
void mp_hal_set_interrupt_char(int c);
static inline void mp_hal_delay_us_fast(uint32_t us) { return; }
