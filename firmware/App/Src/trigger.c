#ifdef DEBUG_LOGGING
#define FILENAME "trigger"
#include "logging.h"
#endif

#include "trigger.h"
#include "led_controller.h"

typedef struct
{
    uint32_t integrator;
    trigger_type_t state;
} trigger_debounce_t;

static const uint32_t INTEGRATOR_MAX = 3;
static trigger_debounce_t slv_state = {0, TRIGGER_RESET};
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
        filtered_state->state = TRIGGER_SET;
    }
    else if (filtered_state->integrator >= INTEGRATOR_MAX)
    {
        filtered_state->state = TRIGGER_RESET;
        filtered_state->integrator = INTEGRATOR_MAX;
    }
    return filtered_state->state != old_state;
}

void trigger_update_input()
{
    GPIO_PinState slv_raw_state = HAL_GPIO_ReadPin(INPUT_SLEEVE_GPIO_Port, INPUT_SLEEVE_Pin);
    GPIO_PinState tip_raw_state = HAL_GPIO_ReadPin(INPUT_TIP_GPIO_Port, INPUT_TIP_Pin);

    bool slv_changed = update_debounce_integrator(slv_raw_state, &slv_state);
    bool tip_changed = update_debounce_integrator(tip_raw_state, &tip_state);

#ifdef DEBUG_LOGGING
    if (slv_changed)
    {
        const char *event_str = slv_state.state == TRIGGER_SET ? "Press" : "Release";
        LOG_DEBUG("SLV %s detected", event_str);
    }

    if (tip_changed)
    {
        const char *event_str = tip_state.state == TRIGGER_SET ? "Press" : "Release";
        LOG_DEBUG("TIP %s detected", event_str);
    }
#endif
}

trigger_type_t trigger_read(trigger_channel_t channel)
{
    if (channel == TRIGGER_CHANNEL_SLEEVE)
    {
        return slv_state.state;
    }
    if (channel == TRIGGER_CHANNEL_TIP)
    {
        return tip_state.state;
    }
    return TRIGGER_RESET;
}

void trigger_set_output(trigger_channel_t channel, trigger_type_t type)
{
    // MCU GPIO state and trigger state are inverted:
    // Pin HIGH = switch open (Trigger reset)
    // Pin LOW = switch closed (Trigger set)
    GPIO_PinState state = GPIO_PIN_RESET;
    if (type == TRIGGER_RESET)
    {
        state = GPIO_PIN_SET;
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