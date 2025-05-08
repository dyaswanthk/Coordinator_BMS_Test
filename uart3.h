#ifndef UART3_H
#define UART3_H

#include "stm32f4xx.h"

#define UART_BUFFER_TX3_SIZE 15
#define UART_BUFFER_RX3_SIZE 15

extern uint8_t uart3_tx[UART_BUFFER_TX3_SIZE];
extern uint8_t uart3_rx[UART_BUFFER_RX3_SIZE];
extern uint8_t uart3_rxFlag;

void uart3_gpio_init(void);
void uart3_dma_init(uint8_t size);
void uart3_init(uint8_t size);
void uart3_send(uint8_t* data, uint8_t size);
void uart3_receive(void);
void uart3_dmaunittest(void);

void rx_enable(void);
void tx_enable(void);

  typedef enum{
    uart_idle,
    uart_busy
  } uart_status_t;


#endif
 