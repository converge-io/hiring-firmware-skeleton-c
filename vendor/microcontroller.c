/**
 * @file microcontroller.c
 * @brief Simulated implementation of a microcontroller interface.
 * 
 * This is a simulation implementation that mimics the behavior of a real
 * microcontroller for testing and development purposes.
 */

#include "microcontroller.h"
#include <stdio.h>
#include <sys/select.h>

void delay_ms(uint16_t ms) {
  struct timeval timeout = {
    .tv_sec = ms / 1000,
    .tv_usec = (ms % 1000) * 1000,
  };
  select(0, NULL, NULL, NULL, &timeout);
}

