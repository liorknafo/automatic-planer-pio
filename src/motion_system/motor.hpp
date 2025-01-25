#pragma once
#include <driver/gpio.h>
#include <driver/ledc.h>

class Motor
{
private:
gpio_num_t in1; ledc_channel_t in1_channel; gpio_num_t in2; ledc_channel_t in2_channel;
public:
    Motor(gpio_num_t in1, ledc_channel_t in1_channel, gpio_num_t in2, ledc_channel_t in2_channel);

    void move_forward(float speed);
    void move(bool forward, float speed);
    void move_backward(float speed);
    void stop();
    void move(float speed);
};