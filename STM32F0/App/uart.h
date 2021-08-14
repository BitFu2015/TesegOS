//uart.h
#ifndef UART_H
#define UART_H

void UartInit(void);
uint8_t UartWrite(uint8_t *buf,uint8_t len);
uint8_t UartRead(uint8_t *buf);

#endif
