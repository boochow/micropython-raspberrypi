#include <unistd.h>
#include "py/mpconfig.h"
#include "mpconfigport.h"
#include "mphalport.h"

#define IOREG(X)  (*(volatile uint32_t *) (X))

// System timer counter
#define SYST_CLO IOREG(0x20003004)
#define SYST_CHI IOREG(0x20003008)

volatile uint64_t systime(void) {
  uint64_t t;
  uint32_t chi;
  uint32_t clo;

  chi = SYST_CHI;
  clo = SYST_CLO;
  if (chi != SYST_CHI) {
    chi = SYST_CHI;
    clo = SYST_CLO;
  }
  t = chi;
  t = t << 32;
  t += clo;
  return t;
}

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

void mp_hal_set_interrupt_char(int c) {
  return;
}
