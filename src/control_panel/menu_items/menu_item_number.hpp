#pragma once
#include "menu_item.hpp"
#include <string>
#include <sstream>
#include <ostream>

template <typename T>
class MenuItemNumber : public MenuItem
{
private:
    std::string name;
    T *value;
    T increment;
    T min;
    T max;

public:
    MenuItemNumber(std::string name, T *value, T increment, T min, T max);

    virtual const std::string get_display_row(int width) override;
    virtual void josystick_handler(JosystickEvent event) override;
};

template <typename T>
MenuItemNumber<T>::MenuItemNumber(std::string name, T *value, T increment, T min, T max)
    : name(name),
      value(value),
      increment(increment),
      min(min),
      max(max)
{
}

template <typename T>
const std::string MenuItemNumber<T>::get_display_row(int width)
{
    std::ostringstream oss;
    oss << this->name;
    for (int i = 0; i < width - this->name.length() - 4; i++)
    {
        oss << " ";
    }
    oss << *this->value;
    return oss.str();
}

template <typename T>
void MenuItemNumber<T>::josystick_handler(JosystickEvent event)
{
    switch (event)
    {
    case JosystickEvent::RIGHT:
        if ((*this->value) + this->increment > this->max)
        {
            *this->value = this->max;
        }
        else
        {
            *this->value += this->increment;
        }
        break;
    case JosystickEvent::LEFT:
        if ((*this->value) - this->increment < this->min)
        {
            *this->value = this->min;
        }
        else
        {
            *this->value -= this->increment;
        }
        break;
    default:
        break;
    }
}