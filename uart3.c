
  #include "uart3.h"
  #include "bms.h"
  #include <stdint.h>
  #include <string.h>

  #include <__cross_studio_io.h>

  uint8_t uart3_tx[UART_BUFFER_TX3_SIZE];
  uint8_t uart3_rx[UART_BUFFER_RX3_SIZE];
  uint8_t uart3_rxFlag;



  uart_status_t uart3_status = uart_idle;

  /*
  UART3 gpio pins are PB10(USART_TX), PB11(USART_RX)
  uart3_dma_init() function configures DMA for uart3
  DMA1 Stream3 channel 4 is configured enable the data in uart3_tx[] to be transferred through uart3
  DMA1 Stream1 channel 4 is configured to transfer the received data from uart2 to uart3_rx[]
  */
  void uart3_gpio_init(void)
  {
      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
      RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

      GPIOB->MODER &= ~GPIO_MODER_MODE11;
      GPIOB->MODER |= (0b10<<GPIO_MODER_MODE11_Pos);

      GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL11;
      GPIOB->AFR[1] |= (0b0111<<GPIO_AFRH_AFSEL11_Pos);

      GPIOB->MODER &= ~GPIO_MODER_MODE10;
      GPIOB->MODER |= (0b10<<GPIO_MODER_MODE10_Pos);

      GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL10;
      GPIOB->AFR[1] |= (0b0111<<GPIO_AFRH_AFSEL10_Pos);

      GPIOD->MODER &= ~GPIO_MODER_MODE11;     //PD11-Receiver enable pin
      GPIOD->MODER |= (0b01<<GPIO_MODER_MODE11_Pos);
      GPIOD->MODER &= ~GPIO_MODER_MODE10;     //PD10-Driver enable pin
      GPIOD->MODER |= (0b01<<GPIO_MODER_MODE10_Pos);
      //GPIOD->BSRR |= GPIO_BSRR_BS11;
      //GPIOD->BSRR |= GPIO_BSRR_BR10;

  }

  void uart3_dma_init(uint8_t size)
  {
      RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
      DMA1_Stream3->CR &= ~DMA_SxCR_CHSEL;            // DMA1-stream3-channel4 corresponds to USART3_TX
      DMA1_Stream3->CR |= (0b100<<DMA_SxCR_CHSEL_Pos);
      //DMA1_Stream3->NDTR = size;

      DMA1_Stream3->PAR = (uint32_t)&(USART3->DR);
      //DMA1_Stream3->M0AR = (uint32_t)&(uart3_tx[0]);

      DMA1_Stream3->CR |= DMA_SxCR_MINC;
      DMA1_Stream3->CR &= ~DMA_SxCR_DIR;
      DMA1_Stream3->CR |= (0b01<<DMA_SxCR_DIR_Pos); // Memory to Peripheral

      DMA1_Stream1->CR &= ~DMA_SxCR_CHSEL;            // DMA1-stream1-channel4 corresponds to USART3_RX
      DMA1_Stream1->CR |= (0b100<<DMA_SxCR_CHSEL_Pos);
      DMA1_Stream1->NDTR = size;

      DMA1_Stream1->PAR = (uint32_t)&(USART3->DR);
      DMA1_Stream1->M0AR = (uint32_t)&(bmsData.rxBuffer[0]);

      DMA1_Stream1->CR |= DMA_SxCR_MINC;
      DMA1_Stream1->CR &= ~DMA_SxCR_DIR; // Pheripheral to Memory
      DMA1_Stream1->CR |= DMA_SxCR_EN;

  }




  void uart3_init(uint8_t size)
  {
      RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

  // Refer to Table 82, page no. 527 (Reference Manual)
  // fPCLK = 42 MHz (APB1 clock frequency is fPCLK)
  // For Baud rate of 9.6 KBps, the value to be programmed is 273.4375 (273 and 7)
      USART3->BRR = 0;
      USART3->BRR |= (273<<USART_BRR_DIV_Mantissa_Pos);
      USART3->BRR |= (7<<USART_BRR_DIV_Fraction_Pos);

      USART3->CR1 = 0;
      USART3->CR2 = 0;
      USART3->CR3 = 0;

      USART3->CR3 |= USART_CR3_DMAR;    // DMA mode is enabled for reception
      USART3->CR3 |= USART_CR3_DMAT;    // DMA mode is enabled for transmission

      USART3->CR2 |= (0b00<<USART_CR2_STOP_Pos);    // 1 Stop bit

      USART3->CR1 &= ~USART_CR1_M;    // 1 Start bit, 8 Data bits, n Stop bit
      USART3->CR1 &= ~USART_CR1_PCE;   // Partity bit disable

      USART3->CR1 |= USART_CR1_UE;    // USART enabled
      USART3->CR1 |= USART_CR1_TE;    // Transmitter is enabled
      USART3->CR1 |= USART_CR1_RE;    // Receiver is enabled and begins searching for a start bit

      
      uart3_gpio_init();

      uart3_dma_init(size);

      uart3_rx[0] = 0;
      uart3_rxFlag = 0;
      USART3->CR1 |= USART_CR1_TCIE; // Transfer Complete Interrupt enabled

      NVIC_EnableIRQ(USART3_IRQn);
      NVIC_SetPriority(USART3_IRQn,2);
  }


  void uart3_send(uint8_t* data, uint8_t size)
  {

      if (uart3_status == uart_busy)
      {
          if (DMA1->LISR & DMA_LISR_TCIF3)
          {
              DMA1->LIFCR |= DMA_LIFCR_CTCIF3;    // Clear the DMA flag
              uart3_status = uart_idle;
              
          }
      } 
      if (uart3_status == uart_idle)
      {
          DMA1_Stream3->M0AR = (uint32_t)data;
          DMA1_Stream3->NDTR = size;
          DMA1_Stream3->CR |= DMA_SxCR_EN;
          uart3_status = uart_busy;
      }
      
      
  }


  void uart3_receive(void)
  { 
     //rx_enable();
      
      if (DMA1->LISR & DMA_LISR_TCIF1)  // Check if DMA transfer happened, indicating uart data reception
      {
        
          uart3_rxFlag = 1;
        
          DMA1->LIFCR |= DMA_LIFCR_CTCIF1;    // Clear the DMA flag
          DMA1_Stream1->CR |= DMA_SxCR_EN;    // Enable DMA for next data to be received
      }
     
  }

  void tx_enable(void)
  {
    GPIOD->BSRR |= GPIO_BSRR_BS11;
    GPIOD->BSRR |= GPIO_BSRR_BS10;
  }
  void rx_enable(void)
  {
    GPIOD->BSRR |= GPIO_BSRR_BR11;
    GPIOD->BSRR |= GPIO_BSRR_BR10;
  }


