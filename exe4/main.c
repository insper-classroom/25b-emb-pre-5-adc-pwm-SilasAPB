/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;

const float conversion_factor = 3.3f / (1 << 12);

/**
 * 0..1.0V: Desligado
 * 1..2.0V: 150 ms
 * 2..3.3V: 400 ms
 */
volatile int timer_led = 0;
volatile int timer_check = 0;

bool timer_0_callback(repeating_timer_t *rt) {
    timer_check = 1;
    return true; // keep repeating
}

bool timer_1_callback(repeating_timer_t *rt) {
    timer_led = 1;
    return true; // keep repeating
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(28);
    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);

    repeating_timer_t timer_0;
    repeating_timer_t timer_1;

    if (!add_repeating_timer_us(10000,
                                timer_0_callback,
                                NULL,
                                &timer_0)) {
        printf("Failed to add timer\n");
    }

    int check_p = 0;
    int aceso = 0;

    while (1) {
        if (timer_check) {
            adc_select_input(2);
            uint16_t result1 = adc_read();
            float volt = result1 * conversion_factor;
            printf("%f\n", volt);
            if (volt > 2 ) {
                if (check_p != 2) {
                    cancel_repeating_timer(&timer_1);
                    if (!add_repeating_timer_us(500000,
                                                timer_1_callback,
                                                NULL,
                                                &timer_1)) {
                        printf("Failed to add timer\n");
                    }
                    check_p = 2;
                }

            } else if (volt > 1) {
                if (check_p != 1) {
                    cancel_repeating_timer(&timer_1);
                    if (!add_repeating_timer_us(300000,
                                                timer_1_callback,
                                                NULL,
                                                &timer_1)) {
                        printf("Failed to add timer\n");
                    }
                    check_p = 1;
                }
            } else if (check_p != 0) {
                cancel_repeating_timer(&timer_1);
            }
            timer_check = 0;
        }
        if (timer_led) {
            gpio_put(PIN_LED_B, !aceso);
            aceso = !aceso;
            timer_led = 0;
        }
    }
}
