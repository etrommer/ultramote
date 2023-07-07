#include <string.h>
#include "app.h"
#include "main.h"
#include "tim.h"
#include "console_impl.h"
#include "logging_impl.h"
#include "logging.h"
#include "bootloader.h"
#include "flash_backup.h"
#include "led_controller.h"
#include "radio.h"
#include "trigger.h"
#include "battery_monitor.h"

static uint32_t last_tick;
static uint8_t buffer[8];

void app_init()
{
    console_impl_init();
    logging_impl_init();
    led_controller_init();
    radio_init();

    led_controller_set_pattern(PATTERN_SOLID);
    led_controller_set_color(256, 1024, 512);

    last_tick = HAL_GetTick();

    // htim17 triggers an interrupt every 10ms
    // to run periodic tasks
    HAL_TIM_Base_Start_IT(&htim17);
}

void app_main_loop()
{
    console_impl_loop();
    battery_monitor_update();
#ifdef ECHO_DEVICE
    if (HAL_GetTick() - last_tick >= 5200)
    {
        // led_controller_set_color(1024, 0, 256);
        // led_controller_set_pattern(PATTERN_BREATHE);
    }

    if (radio_has_data())
    {
        led_controller_set_pattern(PATTERN_SOLID);
        led_controller_set_color(0, 1024, 256);
        // radio_get_data(buffer, &len);
        // radio_send(buffer, len);
        last_tick = HAL_GetTick();
    }
#else
    if (HAL_GetTick() - last_tick >= 5000)
    {
        static uint8_t i = 0;
        buffer[0] = i++;
        // radio_send(buffer, 1);
        last_tick = HAL_GetTick();
    }
#endif // ECHO_DEVICE
}

// Timer callback, should only be called by timer 17
// every 10ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == htim17.Instance)
    {
        // Periodically update LED FSM and button debounce state
        led_controller_update_state();
        trigger_update_input();
    }
}