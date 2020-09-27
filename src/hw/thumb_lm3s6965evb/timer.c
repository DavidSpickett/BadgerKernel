#include "user/timer.h"
#include <stdint.h>

static volatile uint32_t* SYST_CSR = (volatile uint32_t*)0xE000E010;
static volatile uint32_t* SYST_RVR = (volatile uint32_t*)0xE000E014;

static volatile uint32_t* NVIC_ISER0 = (volatile uint32_t*)0xE000E100;
static volatile uint32_t* NVIC_ICER0 = (volatile uint32_t*)0XE000E180;
#define TIMER_INT_BIT 1 << 15

void thumb_enable_timer(void) {
  // Set reload value (23 bits)
  *SYST_RVR = 0x04FFFF;

  uint32_t timer_cfg = *SYST_CSR;
  // Enable timer
  timer_cfg |= 1;
  // Enable tick interrupt generation
  timer_cfg |= 2;
  *SYST_CSR = timer_cfg;

  // Enable handling of the interrupt
  *NVIC_ISER0 |= TIMER_INT_BIT;

  asm volatile("" ::: "memory");
}

void thumb_disable_timer(void) {
  // Looks weird but it is write 1 to disable (different reg though)
  *NVIC_ICER0 |= TIMER_INT_BIT;

  // Stop timer running
  *SYST_CSR &= ~1;

  asm volatile("" ::: "memory");
}
