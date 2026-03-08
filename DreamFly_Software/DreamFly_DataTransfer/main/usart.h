#pragma once

extern const int RX_BUF_SIZE;

void uart_init(void);
int sendData(const char* logName, const char* data);
void uart_tx_data(void *arg);
void uart_rx_data(void *arg);

