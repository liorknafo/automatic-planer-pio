#pragma once
#include "../joystick.hpp"
#include <string>

class MenuItem
{
public:
    virtual const std::string get_display_row(int width) = 0;
    virtual void josystick_handler(JosystickEvent event) {};
    virtual void on_select() {}
    virtual void on_unselect() {}
};