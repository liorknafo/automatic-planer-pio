#pragma once
#include <driver/gpio.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>

class Encoder
{
private:
    gpio_num_t pin;

    void task();

    uint32_t count;

    TaskHandle_t task_handle;

public:
    Encoder(gpio_num_t pin);
    ~Encoder();

    void init();
    int read();
    void reset();
};