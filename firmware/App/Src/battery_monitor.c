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

#define LIPO_VOLTAGE_NUM_POINTS 11

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

uint32_t battery_monitor_percentage_x10()
{
    uint32_t voltage_mv = battery_monitor_mv_value();
    // Voltage-to-percentage lookup table (voltage in mV, charge in tenths of %)
    // Based on typical LiPo discharge curve for a single cell
    const uint16_t lipo_voltage_lut[LIPO_VOLTAGE_NUM_POINTS] = {
        4200, // 100% - Fully charged
        4150, // 90%
        4110, // 80%
        4080, // 70%
        4020, // 60%
        3980, // 50%
        3950, // 40%
        3870, // 30%
        3830, // 20%
        3790, // 10%
        3500  // 0% - Cutoff voltage
    };
    // Clamp to valid range
    if (voltage_mv >= lipo_voltage_lut[0])
    {
        return 1000;
    }
    if (voltage_mv <= lipo_voltage_lut[LIPO_VOLTAGE_NUM_POINTS - 1])
    {
        return 0;
    }

    // Find the two points to interpolate between
    for (uint8_t i = 0; i < LIPO_VOLTAGE_NUM_POINTS - 1; i++)
    {
        if (voltage_mv <= lipo_voltage_lut[i] && voltage_mv >= lipo_voltage_lut[i + 1])
        {
            // Linear interpolation using fixed-point math
            // charge = c1 + (voltage_mv - v1) * (c2 - c1) / (v2 - v1)
            // Charge decreases by 100 (10%) for each step: c1 = 1000 - i*100, c2 = 1000 - (i+1)*100
            uint16_t v1 = lipo_voltage_lut[i];
            uint16_t v2 = lipo_voltage_lut[i + 1];
            uint16_t c1 = 1000 - (i * 100);
            uint16_t c2 = 1000 - ((i + 1) * 100);

            int16_t charge_x10 = c1 + ((int32_t)(voltage_mv - v1) * (int16_t)(c2 - c1)) / (int16_t)(v2 - v1);
            return (uint16_t)charge_x10;
        }
    }

    return 0; // Should never reach here
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
