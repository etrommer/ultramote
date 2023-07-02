#include "trigger.h"
#include "led_controller.h"

typedef struct
{
    uint32_t integrator;
    trigger_type_t state;
} trigger_debounce_t;

static const uint32_t INTEGRATOR_MAX = 3;
static trigger_debounce_t sleeve_state = {0, TRIGGER_RESET};
static trigger_debounce_t tip_state = {0, TRIGGER_RESET};

static bool update_debounce_integrator(GPIO_PinState raw_state, trigger_debounce_t *filtered_state)
{
    trigger_type_t old_state = filtered_state->state;
    if (raw_state == GPIO_PIN_RESET)
    {
        if (filtered_state->integrator > 0)
        {
            filtered_state->integrator -= 1;
        }
    }
    else if (filtered_state->integrator < INTEGRATOR_MAX)
    {
        filtered_state->integrator += 1;
    }

    if (filtered_state->integrator == 0)
    {
        filtered_state->state = TRIGGER_RESET;
    }
    else if (filtered_state->integrator >= INTEGRATOR_MAX)
    {
        filtered_state->state = TRIGGER_SET;
        filtered_state->integrator = INTEGRATOR_MAX;
    }
    return filtered_state->state;
}

void trigger_update_input()
{
    GPIO_PinState sleeve_raw_state = HAL_GPIO_ReadPin(INPUT_SLEEVE_GPIO_Port, INPUT_SLEEVE_Pin);
    GPIO_PinState tip_raw_state = HAL_GPIO_ReadPin(INPUT_TIP_GPIO_Port, INPUT_TIP_Pin);

    update_debounce_integrator(sleeve_raw_state, &sleeve_state);
    update_debounce_integrator(tip_raw_state, &tip_state);

    if (sleeve_state.state == TRIGGER_SET)
    {
        // led_controller_set_color(1023, 0, 0);
    }
    else if (tip_state.state == TRIGGER_SET)
    {
        // led_controller_set_color(0, 1023, 0);
    }
    else
    {
        // led_controller_set_color(0, 0, 1023);
    }
}

trigger_type_t trigger_read(trigger_channel_t channel)
{
    if (channel == TRIGGER_CHANNEL_SLEEVE)
    {
        return sleeve_state.state;
    }
    if (channel == TRIGGER_CHANNEL_TIP)
    {
        return tip_state.state;
    }
    return TRIGGER_RESET;
}

void trigger_set_output(trigger_channel_t channel, trigger_type_t type)
{
    GPIO_PinState state = GPIO_PIN_SET;
    if (type == TRIGGER_RESET)
    {
        state = GPIO_PIN_RESET;
    }

    switch (channel)
    {
    case TRIGGER_CHANNEL_SLEEVE:
        HAL_GPIO_WritePin(OUTPUT_SLEEVE_GPIO_Port, OUTPUT_SLEEVE_Pin, state);
        break;
    case TRIGGER_CHANNEL_TIP:
        HAL_GPIO_WritePin(OUTPUT_TIP_GPIO_Port, OUTPUT_TIP_Pin, state);
        break;
    default:
        break;
    }
}