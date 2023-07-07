#include "led_controller.h"
#include "stdbool.h"
#include "tim.h"

#define CH_RED TIM_CHANNEL_2
#define CH_BLUE TIM_CHANNEL_1
#define CH_GREEN TIM_CHANNEL_3

#define TIMER_MAX 1024

// Internal handling structs
typedef struct
{
    uint32_t r;
    uint32_t g;
    uint32_t b;
} led_color_t;

typedef struct
{
    led_color_t color;
    uint32_t wait_cycles;
    uint32_t fsm_counter;
} led_control_state_t;

// The currently active pattern
static led_pattern_t current_pattern = PATTERN_BREATHE;

// Keeps track of currently selected color
// is copied into the output state by the respective
// pattern functions
static led_color_t current_color = {
    .r = TIMER_MAX,
    .g = 0,
    .b = 0};

static led_control_state_t output_state = {
    // Running counter that is used to track progress in
    // a pattern across invocations
    .fsm_counter = 0,
    // Wait time until next invocation of pattern update function
    // in 10ms increments
    .wait_cycles = 0,
    // next color to be output to LED
    .color = {
        .r = 0,
        .g = 0,
        .b = 0}};

// Slow fading
static void pattern_breathe(led_control_state_t *next_state)
{
    uint32_t brightness;
    const uint32_t total_states = 256;

    // 0-127 = Fade up
    if (next_state->fsm_counter < total_states / 2)
    {
        brightness = next_state->fsm_counter;
    }
    // 128-255 = Fade down
    else
    {
        brightness = (total_states - 1) - next_state->fsm_counter;
    }

    // Square to get visually linear fading
    brightness *= brightness;
    brightness /= 16;

    next_state->color.r = (brightness * current_color.r) / TIMER_MAX;
    next_state->color.g = (brightness * current_color.g) / TIMER_MAX;
    next_state->color.b = (brightness * current_color.b) / TIMER_MAX;
    next_state->wait_cycles = 2;
    next_state->fsm_counter = (next_state->fsm_counter + 1) % total_states;
}

// Aircraft-like blink pattern
// 1. 50ms on
// 2. 50ms off
// 3. 50ms on
// 4. 850ms off
static void pattern_flash(led_control_state_t *next_state)
{
    const uint32_t total_states = 4;

    // Set to default: 50ms Delay, LED off
    next_state->color.r = 0;
    next_state->color.g = 0;
    next_state->color.b = 0;
    next_state->wait_cycles = 5;

    // Turn LED on in states 0 and 2
    if (next_state->fsm_counter == 0 || next_state->fsm_counter == 2)
    {
        next_state->color.r = current_color.r;
        next_state->color.g = current_color.g;
        next_state->color.b = current_color.b;
    }

    // Extend delay for last state
    else if (next_state->fsm_counter == 3)
    {
        next_state->wait_cycles = 85;
    }
    next_state->fsm_counter = (next_state->fsm_counter + 1) % total_states;
}

// Simple, quick blink pattern (100ms on, 100ms off)
static void pattern_quick_blink(led_control_state_t *next_state)
{
    const uint32_t total_states = 2;

    // Set to default: 50ms Delay, LED off
    next_state->wait_cycles = 10;

    // Turn LED on in states 0 and 2
    if (next_state->fsm_counter == 0)
    {
        next_state->color.r = current_color.r;
        next_state->color.g = current_color.g;
        next_state->color.b = current_color.b;
    }
    else
    {
        next_state->color.r = 0;
        next_state->color.g = 0;
        next_state->color.b = 0;
    }
    next_state->fsm_counter = (next_state->fsm_counter + 1) % total_states;
}

// Set a single, constant color
static void pattern_solid(led_control_state_t *next_state)
{
    next_state->color.r = current_color.r;
    next_state->color.g = current_color.g;
    next_state->color.b = current_color.b;
    next_state->wait_cycles = 10;
}

// Set a pattern
void led_controller_set_pattern(led_pattern_t pattern)
{
    if (pattern != current_pattern)
    {
        current_pattern = pattern;
        output_state.fsm_counter = 0;
        output_state.wait_cycles = 0;
    }
}

void led_controller_set_color(uint32_t red, uint32_t green, uint32_t blue)
{
    if (red > TIMER_MAX)
    {
        red = TIMER_MAX;
    }
    if (green > TIMER_MAX)
    {
        green = TIMER_MAX;
    }
    if (blue > TIMER_MAX)
    {
        blue = TIMER_MAX;
    }
    current_color.r = red;
    current_color.g = green;
    current_color.b = blue;
}

// Top-level function that is called after `current_state.wait_cycles` have passed
// to update the LED output according to the current configuration
static void update_pattern_fsm(led_control_state_t *next_state)
{
    switch (current_pattern)
    {
    case PATTERN_FLASH:
        pattern_flash(next_state);
        break;
    case PATTERN_BREATHE:
        pattern_breathe(next_state);
        break;
    case PATTERN_QUICK_BLINK:
        pattern_quick_blink(next_state);
        break;
    default:
        pattern_solid(next_state);
        break;
    }
}

void led_controller_init()
{
    // Set up PWM timer for LED
    __HAL_TIM_SET_COMPARE(&htim2, CH_RED, 0);
    __HAL_TIM_SET_COMPARE(&htim2, CH_GREEN, 0);
    __HAL_TIM_SET_COMPARE(&htim2, CH_BLUE, 0);

    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_PWM_Start(&htim2, CH_RED);
    HAL_TIM_PWM_Start(&htim2, CH_GREEN);
    HAL_TIM_PWM_Start(&htim2, CH_BLUE);
}

void led_controller_update_state()
{
    // Check if an update of the output values is required
    if (output_state.wait_cycles == 0)
    {
        // Get next state
        // This also updates the wait cycles to
        // the next period that is required by the pattern.
        update_pattern_fsm(&output_state);

        // Write new color values to LED PWM timers
        __HAL_TIM_SET_COMPARE(&htim2, CH_RED, output_state.color.r);
        __HAL_TIM_SET_COMPARE(&htim2, CH_GREEN, output_state.color.g);
        __HAL_TIM_SET_COMPARE(&htim2, CH_BLUE, output_state.color.b);
    }
    else
    {
        // No update required
        output_state.wait_cycles -= 1;
    }
}