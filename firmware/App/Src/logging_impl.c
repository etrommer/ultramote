#include "logging.h"
#include "logging_impl.h"
#include "console_config.h"
#include "uart_buffer.h"
#include "main.h"

void logging_print(const char *str)
{
    // Logging messages appear after the console prompt
    // to make the result look less strange, write out
    // the console prompt again after each log line
    uart_write("\33[2K\r");
    uart_write(str);
    uart_write(CONSOLE_PROMPT);
}

void logging_impl_init()
{
    const logging_init_t logging_impl = {
        .default_level = LOGGING_LEVEL_DEBUG,
        .lock_function = NULL,
        .write_function = logging_print,
        .time_ms_function = HAL_GetTick

    };
    logging_init(&logging_impl);
}
