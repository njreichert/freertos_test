#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <hardware/pwm.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <queue.h>

#include <pico/cyw43_arch.h>

#define GPIO_PIN 21
#define DELAY_TIME 20

#define MAX_INPUT_VAL 100
#define MIN_INPUT_VAL -MAX_INPUT_VAL

#define PWM_MIN_VAL 2500 // 1 ms
#define PWM_SPAN ((PWM_MAX_VAL - PWM_MIN_VAL) / 2)
#define PWM_NEUTRAL_VAL (PWM_MIN_VAL + PWM_SPAN) // 1.5 ms
#define PWM_MAX_VAL 5000 // 2 ms

static pwm_config motor_control_cfg;


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


static inline int32_t clamp_i32(int32_t min, int32_t input, int32_t max)
{
    if (input > max) {
        return max;
    } else if (input < min) {
        return min;
    } else {
        return input;
    }
}

void set_pwm_motor_speed(uint pin_num, int32_t speed)
{
    // PWM controllers expect a pulse of anywhere from
    // 1 ms to 2 ms, with a "neutral" pulse being 1.5 ms. 
    // Since we have a clock frequency of 20 Hz, assume a
    // maximum ON period of 5000 counts (50000 counts per
    // period).
    // expected input range: -100 to 100. Input will be
    // clamped to within that range.

    int32_t clamped_speed = clamp_i32(MIN_INPUT_VAL, speed, MAX_INPUT_VAL);

    int32_t level = PWM_NEUTRAL_VAL + ((speed * PWM_SPAN) / MAX_INPUT_VAL);

    uint16_t clamped_level = clamp_i32(PWM_MIN_VAL, level, PWM_MAX_VAL);

    pwm_set_gpio_level(pin_num, clamped_level);
}

pwm_config init_pwm(void) {
    gpio_set_function(GPIO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(GPIO_PIN);
    pwm_config config = pwm_get_default_config();

    // Per Pi RP2040 Datasheet:
    // "Each slice has a fractional clock divider, configured by the DIV
    // register. This is an 8 integer bit, 4 fractional bit clock divider,
    // which allows the count rate to be slowed by up to a factor of 256. 
    // The clock divider allows much lower output frequencies to be achieved
    // — approximately 7.5Hz from a 125MHz system clock."
    //
    // We want a 50 Hz PWM signal using a nominally 150 MHz clock. We set
    // The TOP value to 49999 (i.e.: 50000 counts per period). We then can 
    // Set the Divider by solving for DIV:
    //
    // f_pwm = f_sys / ((TOP + 1) * (CSR_PH_CORRECT + 1) * (DIV))
    // 
    // (Where CSR_PH_CORRECT == 0, f_sys == 150_000_000, f_pwm == 50)
    // This leads to a divider of exactly 60.

    pwm_config_set_wrap(&config, 49999);
    pwm_config_set_clkdiv(&config, 60.0f);

    pwm_init(slice_num, &config, true);

    return config;
}


void task_io(void * args [[maybe_unused]]) {
    while (true) {
        printf("Hello, world!\n");
        vTaskDelay(100);
    }
}


int main(void) {
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    motor_control_cfg = init_pwm();
    TaskHandle_t task_io_handle;

    xTaskCreate(task_io, "task_io", 1024, NULL, 1, &task_io_handle);

    vTaskStartScheduler();

    while (true) {
        // Nothing.
    }

    // set_pwm_motor_speed(GPIO_PIN, 100);

    // while (true) {
    //     int speed = 0;

    //     printf("Please enter a speed: ");
    //     scanf("%d\n", &speed);

    //     printf("Speed is %d\n", speed);
    //     // printf("Please enter a speed: \n");
    //     // scanf("%d\n", &speed);
    //     // set_pwm_motor_speed(GPIO_PIN, -100);

    //     // sleep_ms(5000);

    //     // set_pwm_motor_speed(GPIO_PIN, 100);

    //     // sleep_ms(5000);
        


    //     // // Exercise all possible values.
    //     // for (int32_t i = -100; i <= 100; i++) {
    //     //     set_pwm_motor_speed(GPIO_PIN, i);
    //     //     sleep_ms(DELAY_TIME);
    //     // }
    // }
}
