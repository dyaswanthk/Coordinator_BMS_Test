#ifndef uart3_H
#define uart3_H

#include "stm32f4xx.h"

#define UART_BUFFER_TX3_SIZE 15
#define UART_BUFFER_RX3_SIZE 15

extern uint8_t uart3_tx[UART_BUFFER_TX3_SIZE];
extern uint8_t uart3_rx[UART_BUFFER_RX3_SIZE];
extern uint8_t uart3_rxFlag;

void rx_enable(void);
void tx_enable(void);
void uart3_gpio_init(void);
void uart3_dma_init(uint8_t size);
void uart3_init(uint8_t size);
void uart3_send(uint8_t* data, uint8_t size);
void uart3_receive(void);

#endif
 