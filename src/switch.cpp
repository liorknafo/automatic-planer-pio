#include "switch.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include <iostream>

static const char* TAG = "Switch";

Switch::Switch(gpio_num_t pin, uint32_t debounce_time_ms) 
    : pin(pin)
    , task_handle(nullptr)
    , on_press_callbacks()
    , on_release_callbacks()
    , debounce_time_ms(debounce_time_ms)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << this->pin,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

Switch::~Switch()
{
    if (this->task_handle != nullptr) {
        vTaskDelete(this->task_handle);
        this->task_handle = nullptr;
    }
}

void Switch::start_task_if_needed()
{
    if (this->task_handle != nullptr) {
        return;
    }

    BaseType_t task_created = xTaskCreate(
        [](void *param) { ((Switch *)param)->task(); },
        "button_task",
        2048,
        this,
        5,
        &this->task_handle
    );

    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create button task for pin %d", this->pin);
    }
}

void Switch::task()
{
    bool last_state = false;
    int64_t last_change_time = esp_timer_get_time();
    
    while (true)
    {
        bool current_state = gpio_get_level(this->pin) == 1;
        int64_t current_time = esp_timer_get_time();
        
        if (current_state != last_state) {
            if ((current_time - last_change_time) / 1000 >= this->debounce_time_ms) {
                if (!last_state && current_state)  // Press detected
                {
                    for (const auto &callback : this->on_press_callbacks)
                    {
                        callback();
                    }
                }
                else if (last_state && !current_state)  // Release detected
                {
                    for (const auto &callback : this->on_release_callbacks)
                    {
                        callback();
                    }
                }
                last_state = current_state;
                last_change_time = current_time;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Switch::add_on_press_callback(std::function<void()> callback)
{
    this->on_press_callbacks.push_back(callback);
    this->start_task_if_needed();
}

void Switch::add_on_release_callback(std::function<void()> callback)
{
    this->on_release_callbacks.push_back(callback);
    this->start_task_if_needed();
}

void Switch::wait_for(bool target_state)
{
    while (gpio_get_level(this->pin) != target_state)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool Switch::get() const
{
    return gpio_get_level(this->pin) == 1;
}