#pragma once
#include <driver/gpio.h>
#include <stdint.h>
#include "../control_panel/switch.hpp"

class Encoder
{
private:
    Switch button;
    uint32_t count;

public:
    Encoder(gpio_num_t pin);
    
    void init();
    int read();
    void reset();
};