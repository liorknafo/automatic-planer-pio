#include "menu_item_motor_control.hpp"

MenuItemMotorControl::MenuItemMotorControl(MotionSystem& motion_system)
    : motion_system(motion_system)
{
}

const std::string MenuItemMotorControl::get_display_row(int width)
{
    return "Motor Control";
}

void MenuItemMotorControl::on_select()
{
    this->motion_system.move_by_joystick();
}

void MenuItemMotorControl::on_unselect()
{
    this->motion_system.stop_move_by_joystick();
}
