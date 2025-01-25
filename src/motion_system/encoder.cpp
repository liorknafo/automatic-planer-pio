#include "encoder.hpp"

Encoder::Encoder(gpio_num_t pin) : button(pin), count(0)
{
    this->button.add_on_press_callback([this]() {
        this->count++;
    });
    this->button.add_on_release_callback([this]() {
        this->count++;
    });
}

int Encoder::read()
{
    return this->count;
}

void Encoder::reset()
{
    this->count = 0;
}
