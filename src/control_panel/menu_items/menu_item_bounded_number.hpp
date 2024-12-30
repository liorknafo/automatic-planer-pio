#pragma once
#include "menu_item.hpp"
#include "motion_system/bounded_number.hpp"
#include <string>
#include <sstream>
#include <ostream>
#include <memory>

template <typename T>
class MenuItemBoundedNumber : public MenuItem
{
private:
    std::string name;
    std::shared_ptr<BoundedNumber<T>> value;
    T increment;

public:
    MenuItemBoundedNumber(std::string name, std::shared_ptr<BoundedNumber<T>> value, T increment)
        : name(name),
          value(value),
          increment(increment)
    {
    }

    virtual const std::string get_display_row(int width) override
    {
        std::ostringstream oss;
        oss << this->name;
        for (int i = 0; i < width - this->name.length() - 4; i++)
        {
            oss << " ";
        }
        oss << this->value->get();
        return oss.str();
    }

    virtual void josystick_handler(JosystickEvent event) override
    {
        switch (event)
        {
        case JosystickEvent::RIGHT:
            this->value->increment(this->increment);
            break;
        case JosystickEvent::LEFT:
            this->value->decrement(this->increment);
            break;
        default:
            break;
        }
    }
}; 