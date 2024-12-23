#include <stdio.h>
#include "pico/stdlib.h"

#include <FreeRTOS.h>
#include <task.h>

void configure_timer_for_stats(void)
{
    // Nothing
}

static configRUN_TIME_COUNTER_TYPE curr_time = 0UL;
configRUN_TIME_COUNTER_TYPE get_timer_val(void)
{
    curr_time = timer_hw->timelr;
    return curr_time;
}

void main_task(void *args)
{
    uint32_t delay = *(uint32_t *)(args + 0);

    while (true) {
        printf("Hello, world, %u!\n", delay);
        vTaskDelay(delay);
    }
}

int main()
{
    stdio_init_all();

    uint32_t delay_1 = 1000;
    uint32_t delay_2 = 2000;

    xTaskCreate(main_task, "1000", 4096, &delay_1, 1, NULL);
    xTaskCreate(main_task, "2000", 4096, &delay_2, 1, NULL);

    vTaskStartScheduler();
}
