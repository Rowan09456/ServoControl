#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"

#define TOP 37500 
#define DIV 80.0f
#define SERVO_MIN 1875
#define SERVO_MAX 3750
#define PWM_PIN 0

static int high_count = SERVO_MIN; // 5% duty cycle 
static uint slice_num = 0;


static void __not_in_flash_func(on_pwm_wrap)(void) {
    pwm_clear_irq(slice_num);
    pwm_set_gpio_level(PWM_PIN, high_count); // top 8 bits = table index
}

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    
    //GPIO 0 allocated to the PWM
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    
    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    slice_num = pwm_gpio_to_slice_num(PWM_PIN);

    // Get default configuration for the PWM slice and set the clock divider
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, DIV); // (1.5 Mhz/(80*37500) = 50 Hz)

    // Set period of 37500 cycles (0 to 37499 inclusive)
    pwm_set_wrap(slice_num, TOP); 
	pwm_init(slice_num, &cfg, false);

    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_set_enabled(slice_num, true);

    printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    int i=0;
    
    while (true) {
        printf("Hello, world [%d]!\n", high_count);
        sleep_ms(1000);
            // Set channel A output high for one cycle before dropping
            pwm_set_chan_level(slice_num, PWM_CHAN_A, high_count);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN)); // Toggle LED state
        i++;
        high_count += i*75; // Increase duty cycle by 4% each time

        if(high_count > SERVO_MAX) 
        {
            high_count = SERVO_MIN; // Reset to 5% duty cycle
            i=0;
        }
    }
}


