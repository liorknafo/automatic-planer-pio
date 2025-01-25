#pragma once
#include "menu_item.hpp"
#include "motion_system/motion_system.hpp"
#include <memory>

class MenuItemMotorControl : public MenuItem
{
private:
    MotionSystem& motion_system;

public:
    MenuItemMotorControl(MotionSystem& motion_system);
    virtual const std::string get_display_row(int width) override;
    virtual void on_select() override;
    virtual void on_unselect() override;
};