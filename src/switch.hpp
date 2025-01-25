#pragma once
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include <vector>
#include <functional>

class Switch
{
private:
    gpio_num_t pin;
    TaskHandle_t task_handle;
    std::vector<std::function<void()>> on_press_callbacks;
    std::vector<std::function<void()>> on_release_callbacks;
    uint32_t debounce_time_ms;

    void task();
    void start_task_if_needed();

public:
    Switch(gpio_num_t pin, uint32_t debounce_time_ms = 50);
    ~Switch();

    void add_on_press_callback(std::function<void()> callback);
    void add_on_release_callback(std::function<void()> callback);
    void wait_for(bool pressed);
    bool get() const;
};