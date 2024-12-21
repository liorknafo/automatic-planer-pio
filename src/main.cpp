#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <iostream>

#include "motion_system/motion_system.hpp"

#include "control_panel/menu_items/menu_item_number.hpp"
#include "control_panel/lcd1602.hpp"
#include "control_panel/joystick.hpp"
#include "control_panel/menu.hpp"
#include "control_panel/button.hpp"

#define HOME_PIN GPIO_NUM_27
#define START_PIN GPIO_NUM_25
#define STOP_PIN GPIO_NUM_32

#define X_CHANNEL ADC1_CHANNEL_6
#define Y_CHANNEL ADC1_CHANNEL_7
#define BUTTON_JOYSTICK GPIO_NUM_4

#define I2C_MASTER_SCL_IO GPIO_NUM_22 // I²C clock pin
#define I2C_MASTER_SDA_IO GPIO_NUM_21 // I²C data pin
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000 // I²C frequency
#define LCD_NUM_ROWS 2
#define LCD_NUM_COLUMNS 16
#define LCD_ADDR 0x27 // Adjust based on your I²C adapter address

auto xMotor = Motor(GPIO_NUM_26, LEDC_CHANNEL_0, GPIO_NUM_18, LEDC_CHANNEL_1);
auto yMotor = Motor(GPIO_NUM_19, LEDC_CHANNEL_2, GPIO_NUM_23, LEDC_CHANNEL_3);
auto motion_system = MotionSystem(
    xMotor,
    yMotor,
    GPIO_NUM_2,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_33,
    GPIO_NUM_5);

auto lcd = LCD1602(LCD_ADDR,
                   LCD_NUM_ROWS,
                   LCD_NUM_COLUMNS,
                   I2C_MASTER_SCL_IO,
                   I2C_MASTER_SDA_IO,
                   I2C_MASTER_NUM,
                   I2C_MASTER_FREQ_HZ);

auto joystick = Joystick(X_CHANNEL, Y_CHANNEL, BUTTON_JOYSTICK, 600);
auto menu = Menu(lcd);

auto homeButton = Button(HOME_PIN);
auto startButton = Button(START_PIN);
auto stopButton = Button(STOP_PIN);

extern "C" void app_main()
{
    lcd.init_master();
    lcd.init();
    joystick.start_task();

    menu.add_menu_item(new MenuItemNumber<float>("Cut Speed", &motion_system.cutting_speed, 0.01, 0.01, 1));
    menu.add_menu_item(new MenuItemNumber<float>("Trv Speed", &motion_system.travel_speed, 0.01, 0.01, 1));
    menu.add_menu_item(new MenuItemNumber<uint32_t>("Y Offset", &motion_system.y_offset, 1, 1, 100));
    // float cutting_speed = 0.5;
    // float travel_speed = 1;
    // uint32_t y_offset = 3;
    // menu.add_menu_item(new MenuItemNumber<float>("Cut Speed", &cutting_speed, 0.05, 0.01, 1));
    // menu.add_menu_item(new MenuItemNumber<float>("Trv Speed", &travel_speed, 0.05, 0.01, 1));
    // menu.add_menu_item(new MenuItemNumber<uint32_t>("Y Offset", &y_offset, 1, 1, 100));
    menu.start_task(joystick);
    std::cout << "menu setup" << std::endl;

    motion_system.init();
    homeButton.add_callback([]()
                            { motion_system.home(); });
    homeButton.init();
    startButton.add_callback([]()
                             { motion_system.move(); });
    startButton.init();
    stopButton.add_callback([]()
                            { motion_system.stop(); });
    stopButton.init();
    std::cout << "done setup" << std::endl;
}