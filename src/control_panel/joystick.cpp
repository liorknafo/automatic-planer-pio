#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "joystick.hpp"

#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_ATTEN ADC_ATTEN_DB_12
#define TAG "Joystick"

Joystick::Joystick(adc1_channel_t x_channel, adc1_channel_t y_channel, gpio_num_t button_pin, uint16_t threshold)
    : x_channel(x_channel),
      y_channel(y_channel),
      button_pin(button_pin),
      threshold(threshold),
      callback(std::vector<std::function<void(JosystickEvent)>>()),
      task_handle(nullptr),
      up(false),
      down(false),
      left(false),
      right(false),
      button(false)
{
}

Joystick::~Joystick()
{
    if (this->task_handle != nullptr) {
        vTaskDelete(this->task_handle);
    }
}

void Joystick::start_task()
{
    if (this->task_handle != nullptr) {
        return;  // Task already running
    }

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH));
    ESP_ERROR_CHECK(adc1_config_channel_atten(this->x_channel, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(this->y_channel, ADC_ATTEN));
    ESP_ERROR_CHECK(gpio_set_direction(this->button_pin, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(this->button_pin, GPIO_PULLUP_ONLY));
    
    BaseType_t result = xTaskCreate([](void *joystick)
                { ((Joystick *)joystick)->read_joystick_task(); }, 
                "read_joystick_task", 2048, (void *)this, 5, &this->task_handle);
                
    if (result != pdPASS || this->task_handle == nullptr) {
        ESP_LOGE(TAG, "Failed to create joystick task");
    }
}

void Joystick::read_joystick_task()
{
    while (1)
    {
        int x_raw = adc1_get_raw(this->x_channel);
        int y_raw = adc1_get_raw(this->y_channel);
        
        // Check for invalid ADC readings
        if (x_raw == -1 || y_raw == -1) {
            ESP_LOGE(TAG, "ADC read error");
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        int x = x_raw - 2048;
        int y = y_raw - 2048;
        int button = gpio_get_level(this->button_pin);

        if (y > this->threshold && !this->up)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::UP);
            this->up = true;
        }
        else if (y < this->threshold)
        {
            this->up = false;
        }

        if (y < -this->threshold && !this->down)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::DOWN);
            this->down = true;
        }
        else if (y > -this->threshold)
        {
            this->down = false;
        }

        if (x > this->threshold && !this->right)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::RIGHT);
            this->right = true;
        }
        else if (x < this->threshold)
        {
            this->right = false;
        }

        if (x < -this->threshold && !this->left)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::LEFT);
            this->left = true;
        }
        else if (x > -this->threshold)
        {
            this->left = false;
        }

        if (button == 0 && !this->button)
        {
            for (const auto &cb : this->callback)
                cb(JosystickEvent::BUTTON);
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