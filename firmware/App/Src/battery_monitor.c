#ifdef DEBUG_LOGGING
#define FILENAME "battery_monitor"
#include "logging.h"
#endif

#include "battery_monitor.h"
#include "main.h"
#include "gpio.h"
#include "uart_buffer.h"

typedef enum
{
    IDLE,
    PRECHARGE,
    MEASURE
} battery_monitor_fsm_state_t;

// Voltage divider values: 3V3 -> 10k -> ADC -> 22k -> GND
static const uint32_t VDIV_R_UPPER = 10;
static const uint32_t VDIV_R_LOWER = 22;
static const uint32_t ADC_MAX_VALUE = 4096;

// Measure every 10 seconds
static const uint32_t FREQUENCY = 10000;
// Give voltage over divider 100ms to stabilize
static const uint32_t VDIV_PRECHARGE_TIME = 100;

static volatile uint32_t adc_value = 0;

static uint32_t last_tick = 0;
static battery_monitor_fsm_state_t monitor_state = IDLE;

void battery_monitor_update()
{
    uint32_t current_tick = HAL_GetTick();

    if ((monitor_state == IDLE) && (current_tick - last_tick >= FREQUENCY - VDIV_PRECHARGE_TIME))
    {
        // Enable voltage divider
        HAL_GPIO_WritePin(BATTERY_DIVIDER_ON_GPIO_Port, BATTERY_DIVIDER_ON_Pin, GPIO_PIN_SET);
        monitor_state = PRECHARGE;
        last_tick = current_tick;
    }
    else if ((monitor_state == PRECHARGE) && (current_tick - last_tick >= VDIV_PRECHARGE_TIME))
    {
        // Start measurement
        HAL_ADC_Start_IT(&hadc);
        monitor_state = MEASURE;
        last_tick = current_tick;
    }
}

uint32_t battery_monitor_mv_value()
{
    // Convert ADC Reading to voltage in mV
    uint32_t adc_output = (adc_value * VDD_VALUE) / ADC_MAX_VALUE;
    // Convert ADC voltage to voltage over the voltage divider
    return (adc_output / VDIV_R_LOWER) * (VDIV_R_UPPER + VDIV_R_LOWER);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    // Read & Update The ADC Result
    uint32_t val = HAL_ADC_GetValue(hadc);
    adc_value = val;
    HAL_GPIO_WritePin(BATTERY_DIVIDER_ON_GPIO_Port, BATTERY_DIVIDER_ON_Pin, GPIO_PIN_RESET);
    monitor_state = IDLE;
}
