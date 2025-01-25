#include "motor.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

Motor::Motor(gpio_num_t in1, ledc_channel_t in1_channel, gpio_num_t in2, ledc_channel_t in2_channel) 
    : in1(in1)
    , in1_channel(in1_channel)
    , in2(in2)
    , in2_channel(in2_channel)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t in1_channel_config = {
        .gpio_num = this->in1,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = this->in1_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&in1_channel_config));

    ledc_channel_config_t in2_channel_config = {
        .gpio_num = this->in2,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = this->in2_channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&in2_channel_config));
}

void Motor::move(bool forward, float speed)
{
    if (forward)
    {
        this->move_forward(speed);
    }
    else
    {
        this->move_backward(speed);
    }
}

void Motor::move(float speed)
{
    if (speed > 0)
    {
        this->move_forward(speed);
    }
    else if (speed < 0)
    {
        this->move_backward(-speed);
    }
    else
    {
        this->stop();
    }
}

void Motor::move_backward(float speed)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, this->in1_channel, speed * 8191));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, this->in1_channel));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, this->in2_channel, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, this->in2_channel));
}

void Motor::move_forward(float speed)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, this->in1_channel, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, this->in1_channel));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, this->in2_channel, speed * 8191));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, this->in2_channel));
}

void Motor::stop()
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, this->in1_channel, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, this->in1_channel));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, this->in2_channel, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, this->in2_channel));
}
