/*
 * MIT License
 *
 * Copyright (c) 2018 David Antliff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file app_main.c
 * @brief Example application for the ESP32 Frequency Counter component.
 *
 * Includes an example of re-routing a GPIO input to an output, in this case
 * making an external LED show the state of the signal being measured.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "frequency_count.h"

#define TAG "app"

#define GPIO_SIGNAL_INPUT (CONFIG_FREQ_SIGNAL_GPIO)
#define GPIO_RMT_GATE     (CONFIG_SAMPLING_WINDOW_GPIO)
#define GPIO_LED          (2) //CONFIG_LED_GPIO)

// internal signals for GPIO constant levels
#define GPIO_CONSTANT_LOW   0x30
#define GPIO_CONSTANT_HIGH  0x38

#define PCNT_UNIT         (0)
#define PCNT_CHANNEL      (PCNT_CHANNEL_0)
#define RMT_CHANNEL       (RMT_CHANNEL_0)
#define RMT_MAX_BLOCKS    (2)   // allow up to 2 * 64 * (2 * 32767) RMT periods in window
#define RMT_CLK_DIV       160   // results in 2us steps (80MHz / 160 = 0.5 MHz
//#define RMT_CLK_DIV       20    // results in 0.25us steps (80MHz / 20 = 4 MHz
//#define RMT_CLK_DIV       1     // results in 25ns steps (80MHz / 2 / 1 = 40 MHz)

#define SAMPLE_PERIOD 12.0  // seconds

// The counter is signed 16-bit, so maximum positive value is 32767
// The filter is unsigned 10-bit, maximum value is 1023. Use full period of maximum frequency.
// For higher expected frequencies, the sample period and filter must be reduced.

// suitable up to 16,383.5 Hz
#define WINDOW_DURATION 10.0  // seconds
#define FILTER_LENGTH 1023  // APB @ 80MHz, limits to < 39,100 Hz

// suitable up to 163,835 Hz
//#define WINDOW_LENGTH 0.1  // seconds
//#define FILTER_LENGTH 122  // APB @ 80MHz, limits to < 655,738 Hz

// suitable up to 1,638,350 Hz
//#define WINDOW_LENGTH 0.01  // seconds
//#define FILTER_LENGTH 12  // APB @ 80MHz, limits to < 3,333,333 Hz

// suitable up to 16,383,500 Hz - no filter
//#define WINDOW_LENGTH 0.001  // seconds
//#define FILTER_LENGTH 0  // APB @ 80MHz, limits to < 40 MHz

static void window_start_callback(void)
{
    ESP_LOGI(TAG, "Begin sampling");
    gpio_matrix_in(GPIO_SIGNAL_INPUT, SIG_IN_FUNC228_IDX, false);
}

static void frequency_callback(double hz)
{
    gpio_matrix_in(GPIO_CONSTANT_LOW, SIG_IN_FUNC228_IDX, false);
    ESP_LOGI(TAG, "Frequency %f Hz", hz);
}

static void config_led(void)
{
    gpio_pad_select_gpio(GPIO_LED);
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);

    // route incoming frequency signal to onboard LED when sampling
    gpio_matrix_out(GPIO_LED, SIG_IN_FUNC228_IDX, false, false);
}

void app_main()
{
    config_led();

    frequency_count_configuration_t * config = malloc(sizeof(*config));
    config->pcnt_gpio = GPIO_SIGNAL_INPUT;
    config->pcnt_unit = PCNT_UNIT;
    config->pcnt_channel = PCNT_CHANNEL;
    config->rmt_gpio = GPIO_RMT_GATE;
    config->rmt_channel = RMT_CHANNEL;
    config->rmt_clk_div = RMT_CLK_DIV;
    config->rmt_max_blocks = 2;
    config->sampling_period_seconds = SAMPLE_PERIOD;
    config->sampling_window_seconds = WINDOW_DURATION;
    config->filter_length = FILTER_LENGTH;
    config->window_start_callback = &window_start_callback;
    config->frequency_update_callback = &frequency_callback;

    // task takes ownership of allocated memory
    xTaskCreate(&frequency_count_task_function, "frequency_count_task", 4096, config, 5, NULL);
}

