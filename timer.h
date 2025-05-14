#ifndef TIMER_H
#define TIMER_H

#include "stm32f4xx.h"
#include "clock_init.h"

#define ISR_FREQ 10

void timer2_init(void);
void timer_gpio_init(void);

#endif