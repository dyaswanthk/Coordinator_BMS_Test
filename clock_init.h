#ifndef __CLOCK_INIT_H
#define __CLOCK_INIT_H

#include "stm32f4xx.h"

#define AHB_FREQ 168000
#define APB2_PERIPHERAL_FREQ 84000
#define APB1_PERIPHERAL_FREQ 42000
#define APB2_TIMER_FREQ 168000
#define APB1_TIMER_FREQ 84000

void clock_init(void);

#endif