#pragma once
#include <stdint.h>

typedef enum
{
    CH_RED,
    CH_BLUE,
    CH_GREEN
} led_ch_t;

typedef enum
{
    PATTERN_FLASH,
    PATTERN_BREATHE,
    PATTERN_SOLID
} led_pattern_t;

void led_controller_init(void);
void led_controller_set_pattern(led_pattern_t pattern);
void led_controller_set_color(uint32_t red, uint32_t green, uint32_t blue);
void led_controller_update_state(void);
