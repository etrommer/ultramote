#include "uart_buffer.h"
#include "usart.h"
#include "string.h"
#include "console.h"

#define RINGBUFFER_MAX_SIZE 4
#define UART_BUFFER_SIZE CONSOLE_MAX_LINE_LENGTH

uint8_t rb_data[RINGBUFFER_MAX_SIZE][UART_BUFFER_SIZE];
uint8_t rb_lengths[RINGBUFFER_MAX_SIZE];

uint8_t rb_head = 0;
uint8_t rb_tail = 0;
volatile uint8_t rb_size = 0;

void uart_write(const char *str)
{

    // Busy wait if ringbuffer is full
    while (rb_size >= RINGBUFFER_MAX_SIZE)
        ;

    // Count string length
    uint8_t length = strnlen(str, UART_BUFFER_SIZE);

    // Make FIFO update atomic so we can safely call it from Interrupts as well
    __disable_irq();

    // Copy data and update FIFO
    memcpy(rb_data[rb_head], str, length);
    rb_lengths[rb_head] = length;
    rb_size += 1;
    rb_head = (rb_head + 1) % RINGBUFFER_MAX_SIZE;

    __enable_irq();

    // Start the next transfer if we're idling
    if (hdma_usart2_tx.State == HAL_DMA_STATE_READY)
    {
        HAL_UART_Transmit_DMA(&huart2, rb_data[rb_tail], rb_lengths[rb_tail]);
    }
}

bool uart_busy()
{
    return rb_size != 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
    rb_size -= 1;
    rb_tail = (rb_tail + 1) % RINGBUFFER_MAX_SIZE;
    if (rb_size != 0)
    {
        HAL_UART_Transmit_DMA(&huart2, rb_data[rb_tail], rb_lengths[rb_tail]);
    }
}

uint8_t uart_read(char *buf)
{
    uint8_t size = 1;
    HAL_StatusTypeDef status = HAL_UART_Receive(&huart2, buf, size, 100);
    if (status != HAL_OK)
    {
        return 0;
    }
    else
    {
        return size;
    }
}
