#pragma once
#include "main.h"
#include "gpio.h"
#include <stdbool.h>

typedef enum
{
    TRIGGER_SET,
    TRIGGER_RESET
} trigger_type_t;

typedef enum
{
    TRIGGER_CHANNEL_TIP,
    TRIGGER_CHANNEL_SLEEVE
} trigger_channel_t;

void trigger_set_output(trigger_channel_t channel, trigger_type_t type);
void trigger_update_input(void);
trigger_type_t trigger_read(trigger_channel_t channel);