
#include "clock_init.h"


/*
  External crystal - 8MHz
  HSE - 8 MHz
  /M - /4
  Input frequency to PLL - 2MHz (Max allowed frequency of the input to VCO is 2 MHz)
  xN = x168
  VCO output - 336 MHz
  /P - /2
  PLLCLK - 168 MHz
  /Q - /7
  PLL48CK - 48 MHz

Max frequency allowed for: (Table 17 - datasheet)
  AHB - 168 Mhz
  APB2 peripheral clock - 84 MHz
  APB1 peripheral clock - 42 MHz

  Refer to Fig. 16, page no. 152, reference manual 
  AHB prescalar - 1
  APB2 prescalar - 2
  APB1 prescalar - 4

  APB2 timer clock - 168 MHz (x2)
  APB1 timer clock - 84 MHz (x2)


*/


void clock_init()
{
  /*
   * CP10 and CP11 bits of CPACR register must be 0b11 to enable hardware floating-point unit of the MCU
   * Go to Project -> Properties -> Code Generation -> ARM FP ABI Type, and change the option to 'Hard' 
   */
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));

  FLASH->ACR |= FLASH_ACR_PRFTEN;

/*
 * Refer to datasheet, Table 17 (for requirement to achieve 168 MHz)
 */

  RCC->APB1ENR |= RCC_APB1ENR_PWREN;

  PWR->CR &= ~PWR_CR_VOS;

/* 
 * Power scale 1 (VOS='0b11') selected to achieve 168 MHz clock freq for AHBP 
 */
  PWR->CR |= (0b11<<PWR_CR_VOS_Pos);  

  RCC->CR |= RCC_CR_HSEON;
  while(!(RCC->CR & RCC_CR_HSERDY));

  RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE; //hse

  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;
  RCC->PLLCFGR |= (0x4<<RCC_PLLCFGR_PLLM_Pos);

  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;
  RCC->PLLCFGR |= (168<<RCC_PLLCFGR_PLLN_Pos);

  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;
  //RCC->PLLCFGR |= (2<<RCC_PLLCFGR_PLLP_Pos);

  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLQ;
  RCC->PLLCFGR |= (7<<RCC_PLLCFGR_PLLQ_Pos);

  RCC->CR |= RCC_CR_PLLON;
  while(!(RCC->CR & RCC_CR_PLLRDY));

  // latency - 5 Wait state - table 11, page 81 - reference manual  
  // with 168 Mhz clock frequency, 5 wait states are required 
  FLASH->ACR |= (0x5<<FLASH_ACR_LATENCY_Pos);
  while((FLASH->ACR & FLASH_ACR_LATENCY) != 0x5);

  RCC->CFGR &= ~RCC_CFGR_HPRE;

/* According reference manual prescaling values for PPRE2 and PPRE1:
    0xx - divide by 1
    100 - divide by 2
    101 - divide by 4
    110 - divide by 8
    111 - divide by 16
*/
  RCC->CFGR &= ~RCC_CFGR_PPRE2;
  RCC->CFGR |= (0b100<<RCC_CFGR_PPRE2_Pos);

  RCC->CFGR &= ~RCC_CFGR_PPRE1;
  RCC->CFGR |= (0b101<<RCC_CFGR_PPRE1_Pos);


  RCC->CFGR |= (0b10 << RCC_CFGR_SW_Pos);
  while((RCC->CFGR & RCC_CFGR_SWS) != (0b10 << RCC_CFGR_SWS_Pos));

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  SYSCFG->CMPCR = SYSCFG_CMPCR_CMP_PD;
  
  RCC->CSR |= RCC_CSR_RMVF;

}
