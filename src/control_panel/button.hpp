#pragma once
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include <vector>
#include <functional>

class Button
{
private:
    gpio_num_t pin;
    TaskHandle_t task_handle;
    std::vector<std::function<void()>> callbacks;

    void task();

public:
    Button(gpio_num_t pin);
    ~Button();

    void init();
    void add_callback(std::function<void()> callback);
};