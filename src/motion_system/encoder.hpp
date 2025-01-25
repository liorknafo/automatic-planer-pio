#pragma once
#include <driver/gpio.h>
#include <stdint.h>
#include "switch.hpp"

class Encoder
{
private:
    Switch button;
    uint32_t count;

public:
    Encoder(gpio_num_t pin);
    
    int read();
    void reset();
};