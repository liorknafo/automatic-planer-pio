#pragma once
#include <stdio.h>
#include <functional>
#include <vector>
#include "driver/adc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "switch.hpp"
#include <optional>

enum JosystickEvent
{
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    BUTTON = 4
};

class Joystick
{
private:
    adc1_channel_t x_channel;
    adc1_channel_t y_channel;
    Switch button_switch;
    std::vector<std::function<void(JosystickEvent)>> callback;
    TaskHandle_t task_handle;

    bool up;
    bool down;
    bool left;
    bool right;
    bool button;

    float dead_zone;

    void read_joystick_task();

public:
    Joystick(adc1_channel_t x_channel, adc1_channel_t y_channel, gpio_num_t button_switch, float dead_zone = 0.2f);
    ~Joystick();
    void add_callback(std::function<void(JosystickEvent)> callback);
    std::optional<std::pair<float, float>> get_position();
};
