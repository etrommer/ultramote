#pragma once
#include <stdint.h>
#include <stdbool.h>

void uart_write(const char *str);
uint8_t uart_read(char *buf);
bool uart_busy();