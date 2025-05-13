#include "stm32f4xx.h"
#include "uart3.h"
#include "bms.h"
#include "clock_init.h"
#include "timer.h"
#include <string.h>
#include <cross_studio_io.h>



int main(void)
{
 clock_init();
 uart3_init(XFER_BUFFER_LENGTH);
 bmsinit();
 timer2_init();
 while(1)
  {
   uart3_receive();
   if (uart3_rxFlag){
        bmsProcessResponse(bmsData.rxBuffer, XFER_BUFFER_LENGTH );
        uart3_rxFlag = 0;
    }

    }
  
}



void TIM2_IRQHandler(void)
{
  if(TIM2->SR & TIM_SR_UIF)
  {
    TIM2->SR &= ~TIM_SR_UIF;
    bmsCreateTxPacket();
    tx_enable();
    uart3_send(bmsData.txBuffer, XFER_BUFFER_LENGTH);
    
  }
      

  }

void USART3_IRQHandler(void)
{
    // Transmit complete
   
    if (USART3->SR & USART_SR_TC)
    { 
        USART3->SR &= ~USART_SR_TC;          // Clear TC flag
    }
     rx_enable();
}