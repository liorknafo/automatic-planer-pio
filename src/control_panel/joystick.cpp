#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "joystick.hpp"
#include <iostream>

#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_ATTEN ADC_ATTEN_DB_12
#define TAG "Joystick"

Joystick::Joystick(adc1_channel_t x_channel, adc1_channel_t y_channel, gpio_num_t button_pin, float dead_zone)
    : x_channel(x_channel)
    , y_channel(y_channel)
    , button_switch(button_pin)
    , callback(std::vector<std::function<void(JosystickEvent)>>())
    , task_handle(nullptr)
    , up(false)
    , down(false)
    , left(false)
    , right(false)
    , button(false)
    , dead_zone(dead_zone)
{
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH));
    ESP_ERROR_CHECK(adc1_config_channel_atten(this->x_channel, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(this->y_channel, ADC_ATTEN));

    BaseType_t result = xTaskCreate([](void *joystick)
                                    { ((Joystick *)joystick)->read_joystick_task(); },
                                    "read_joystick_task", 8192, (void *)this, 5, &this->task_handle);

    if (result != pdPASS || this->task_handle == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create joystick task");
    }
}

Joystick::~Joystick()
{
    if (this->task_handle != nullptr)
    {
        vTaskDelete(this->task_handle);
    }
}

void Joystick::read_joystick_task()
{
    while (1)
    {
        auto pos = this->get_position();

        // Check for invalid ADC readings
        if (!pos.has_value())
        {
            ESP_LOGE(TAG, "ADC read error");
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        auto [x, y] = *pos;

        bool button_pressed = this->button_switch.get();

        if (y > 0 && !this->up)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::UP);
            this->up = true;
        }
        else if (y <= 0)
        {
            this->up = false;
        }

        if (y < 0 && !this->down)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::DOWN);
            this->down = true;
        }
        else if (y >= 0)
        {
            this->down = false;
        }

        if (x > 0 && !this->right)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::RIGHT);
            this->right = true;
        }
        else if (x <= 0)
        {
            this->right = false;
        }

        if (x < 0 && !this->left)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::LEFT);
            this->left = true;
        }
        else if (x >= 0)
        {
            this->left = false;
        }

        if (button_pressed && !this->button)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::BUTTON);
            this->button = true;
        }
        else if (!button_pressed)
        {
            this->button = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

std::optional<std::pair<float, float>> Joystick::get_position()
{
    int x_raw = adc1_get_raw(this->x_channel);
    int y_raw = adc1_get_raw(this->y_channel);

    if (x_raw == -1 || y_raw == -1)
    {
        ESP_LOGE(TAG, "ADC read error");
        return std::nullopt;
    }

    float x = (x_raw - 2048) / 2048.0f;
    float y = (y_raw - 2048) / 2048.0f;

    // Apply dead zone
    if (std::abs(x) < dead_zone)
        x = 0.0f;
    if (std::abs(y) < dead_zone)
        y = 0.0f;

    return std::make_pair(x, y);
}

void Joystick::add_callback(std::function<void(JosystickEvent)> callback)
{
    this->callback.push_back(callback);
}