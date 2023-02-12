#include <string.h>

#include "console_impl.h"
#include "console.h"
#include "uart_buffer.h"
#include "led_controller.h"
#include "bootloader.h"
#include "trigger.h"
#include "battery_monitor.h"
#include "fw_version.h"

#define INPUT_BUFFER_SIZE 16
#define OUTPUT_BUFFER_SIZE 128

static uint8_t input_buffer[INPUT_BUFFER_SIZE];
static uint8_t output_buffer[OUTPUT_BUFFER_SIZE];

CONSOLE_COMMAND_DEF(status, "Print System Status");
CONSOLE_COMMAND_DEF(bl, "Enter Bootloader");

CONSOLE_COMMAND_DEF(sc, "Set Channel",
                    CONSOLE_INT_ARG_DEF(channel, "Channel to set <0-255>"));

CONSOLE_COMMAND_DEF(slc, "Set LED Color",
                    CONSOLE_INT_ARG_DEF(red, "Red Value <0-1024>"),
                    CONSOLE_INT_ARG_DEF(green, "Green Value <0-1024>"),
                    CONSOLE_INT_ARG_DEF(blue, "Blue Value <0-1024>"));

CONSOLE_COMMAND_DEF(slp, "Set LED Pattern",
                    CONSOLE_STR_ARG_DEF(pattern, "Pattern to show [solid, flash, breathe]"));

CONSOLE_COMMAND_DEF(trg, "Set State of output port",
                    CONSOLE_STR_ARG_DEF(channel, "Trigger Channel [S, T]"),
                    CONSOLE_INT_ARG_DEF(state, "Channel State [0, 1]"));

static void status_command_handler(const status_args_t *args)
{
    // TODO: Read channel
    // TODO: Read charge state
    snprintf(output_buffer, OUTPUT_BUFFER_SIZE, "Firmware Version: %d.%d.%d\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);
    uart_write(output_buffer);

    snprintf(output_buffer, OUTPUT_BUFFER_SIZE, "Battery Voltage: %dmV\n", battery_monitor_mv_value());
    uart_write(output_buffer);

    trigger_type_t in1 = trigger_read(TRIGGER_CHANNEL_SLEEVE);
    snprintf(output_buffer, OUTPUT_BUFFER_SIZE, "Input Ch. 1 (Sleeve): %s\n", in1 == TRIGGER_SET ? "High" : "Low");
    uart_write(output_buffer);

    trigger_type_t in2 = trigger_read(TRIGGER_CHANNEL_TIP);
    snprintf(output_buffer, OUTPUT_BUFFER_SIZE, "Input Ch. 2 (Tip): %s\n", in2 == TRIGGER_SET ? "High" : "Low");
    uart_write(output_buffer);
}

static void bl_command_handler(const bl_args_t *args)
{
    snprintf(output_buffer, OUTPUT_BUFFER_SIZE, "Entering Bootloader...\n");
    uart_write(output_buffer);
    while (uart_busy())
        ;

    bootloader_enter();
}

static void sc_command_handler(const sc_args_t *args)
{
    snprintf(output_buffer, OUTPUT_BUFFER_SIZE, "Set Channel: %d (not implemented)\n", args->channel);
    uart_write(output_buffer);
}

static void slc_command_handler(const slc_args_t *args)
{
    led_controller_set_color(args->red, args->green, args->blue);
}

static void slp_command_handler(const slp_args_t *args)
{
    if (strncmp(args->pattern, "solid", 5) == 0)
    {
        led_controller_set_pattern(PATTERN_SOLID);
    }
    if (strncmp(args->pattern, "breathe", 7) == 0)
    {
        led_controller_set_pattern(PATTERN_BREATHE);
    }
    if (strncmp(args->pattern, "flash", 5) == 0)
    {
        led_controller_set_pattern(PATTERN_FLASH);
    }
}

static void trg_command_handler(const trg_args_t *args)
{
    if (strncmp(args->channel, "S", 1) && args->state == 0)
    {
        trigger_set_output(TRIGGER_CHANNEL_SLEEVE, TRIGGER_RESET);
    }
    else if (strncmp(args->channel, "S", 1) && args->state == 1)
    {
        trigger_set_output(TRIGGER_CHANNEL_SLEEVE, TRIGGER_SET);
    }
    else if (strncmp(args->channel, "T", 1) && args->state == 0)
    {
        trigger_set_output(TRIGGER_CHANNEL_TIP, TRIGGER_RESET);
    }
    else if (strncmp(args->channel, "T", 1) && args->state == 1)
    {
        trigger_set_output(TRIGGER_CHANNEL_TIP, TRIGGER_SET);
    }
}

void console_impl_init()
{
    const console_init_t init_console = {
        .write_function = uart_write,
    };
    console_init(&init_console);
    console_command_register(status);
    console_command_register(bl);
    console_command_register(sc);
    console_command_register(slc);
    console_command_register(slp);
    console_command_register(trg);
}

void console_impl_loop()
{
    uint8_t size = uart_read(input_buffer);
    console_process(input_buffer, size);
}