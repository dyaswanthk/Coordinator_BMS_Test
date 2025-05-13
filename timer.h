#ifndef TIMER_H
#define TIMER_H

#include "stm32f4xx.h"
#include "clock_init.h"

#define ISR_FREQ 1

void timer2_init(void);

#endif