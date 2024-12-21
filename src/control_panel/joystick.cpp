#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "joystick.hpp"

#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_ATTEN ADC_ATTEN_DB_12

Joystick::Joystick(adc1_channel_t x_channel, adc1_channel_t y_channel, gpio_num_t button_pin, uint16_t threshold)
    : x_channel(x_channel),
      y_channel(y_channel),
      button_pin(button_pin),
      threshold(threshold),
      callback(std::vector<std::function<void(JosystickEvent)>>()),
      up(false),
      down(false),
      left(false),
      right(false),
      button(false)
{
}

void Joystick::start_task()
{
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(this->x_channel, ADC_ATTEN);
    adc1_config_channel_atten(this->y_channel, ADC_ATTEN);
    gpio_set_direction(this->button_pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(this->button_pin, GPIO_PULLUP_ONLY);
    xTaskCreate([](void *joystick)
                { ((Joystick *)joystick)->read_joystick_task(); }, "read_joystick_task", 2048, (void *)this, 5, nullptr);
}

void Joystick::read_joystick_task()
{
    while (1)
    {
        int x = adc1_get_raw(this->x_channel) - 2048;
        int y = adc1_get_raw(this->y_channel) - 2048;
        int button = gpio_get_level(this->button_pin);
        if (y > this->threshold && !this->up)
        {
            for (const auto &callback : this->callback)
                callback(JosystickEvent::UP);
            this->up = true;
        }
        else if (y < this->threshold)
        {
            this->up = false;
        }

        if (y < -this->threshold && !this->down)
        {
            for (const auto &callback : this->callback)
                callback(JosystickEvent::DOWN);
            this->down = true;
        }
        else if (y > -this->threshold)
        {
            this->down = false;
        }

        if (x > this->threshold && !this->right)
        {
            for (const auto &callback : this->callback)
                callback(JosystickEvent::RIGHT);
            this->right = true;
        }
        else if (x < this->threshold)
        {
            this->right = false;
        }

        if (x < -this->threshold && !this->left)
        {
            for (const auto &callback : this->callback)
                callback(JosystickEvent::LEFT);
            this->left = true;
        }
        else if (x > -this->threshold)
        {
            this->left = false;
        }

        if (button == 0 && !this->button)
        {
            for (const auto &callback : this->callback)
                callback(JosystickEvent::BUTTON);
            this->button = true;
        }
        else if (button == 1)
        {
            this->button = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Joystick::add_callback(std::function<void(JosystickEvent)> callback)
{
    this->callback.push_back(callback);
}