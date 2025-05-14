#include "timer.h"

void timer2_init(void) {


    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CR1 = 0;
    TIM2->CR1 |= TIM_CR1_ARPE;         // Auto-reload preload enable
    TIM2->CR1 |= (0b00 << TIM_CR1_CMS_Pos);

    TIM2->PSC = 8399;                  // Prescaler (adjust to get desired frequency)
    TIM2->ARR = 9999;  // Auto-reload value
    TIM2->RCR = 0;

    TIM2->EGR |= TIM_EGR_UG;           // Force update to load registers

    TIM2->DIER |= TIM_DIER_UIE;        // Enable update interrupt
    
    TIM2->CR1 |= TIM_CR1_CEN;          // Enable counter
    NVIC_EnableIRQ(TIM2_IRQn);         // Enable TIM2 IRQ in NVIC
    NVIC_SetPriority(TIM2_IRQn, 0);
}

//void timer_gpio_init(void) {

//  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
//  GPIOA->MODER &= ~GPIO_MODER_MODE0;
//  GPIOA->MODER |= (0b01<<GPIO_MODER_MODE0_Pos);
//  GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL0;
//  GPIOA->AFR[0] |= (0b0001<<GPIO_AFRL_AFSEL0_Pos);
//  GPIOA->OTYPER &= ~GPIO_OTYPER_OT0;
//  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;

//}