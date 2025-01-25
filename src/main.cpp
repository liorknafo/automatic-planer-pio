#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <memory>

#include "motion_system/motion_system.hpp"

#include "control_panel/menu_items/menu_item_bounded_number.hpp"
#include "control_panel/menu_items/menu_item_motor_control.hpp"
#include "control_panel/lcd1602.hpp"
#include "control_panel/joystick.hpp"
#include "control_panel/menu.hpp"
#include "switch.hpp"

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

static const char *TAG = "Main";

auto xMotor = Motor(GPIO_NUM_26, LEDC_CHANNEL_0, GPIO_NUM_18, LEDC_CHANNEL_1);
auto yMotor = Motor(GPIO_NUM_19, LEDC_CHANNEL_2, GPIO_NUM_23, LEDC_CHANNEL_3);

auto cutting_speed = std::make_shared<BoundedNumber<float>>(1.0, 0.7, 1.0);
auto travel_speed = std::make_shared<BoundedNumber<float>>(1.0, 0.7, 1.0);
auto y_offset = std::make_shared<BoundedNumber<uint32_t>>(3, 1, 100);

auto joystick = Joystick(X_CHANNEL, Y_CHANNEL, BUTTON_JOYSTICK);

auto motion_system = MotionSystem(
    xMotor,
    yMotor,
    GPIO_NUM_2,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_33,
    GPIO_NUM_5,
    cutting_speed,
    travel_speed,
    y_offset,
    joystick);

auto lcd = LCD1602(LCD_ADDR,
                   LCD_NUM_ROWS,
                   LCD_NUM_COLUMNS,
                   I2C_MASTER_SCL_IO,
                   I2C_MASTER_SDA_IO,
                   I2C_MASTER_NUM,
                   I2C_MASTER_FREQ_HZ);

auto menu = Menu(lcd);

auto homeButton = Switch(HOME_PIN);
auto startButton = Switch(START_PIN);
auto stopButton = Switch(STOP_PIN);

extern "C" void app_main()
{

    menu.add_menu_item(std::make_unique<MenuItemBoundedNumber<float>>("Cut Speed", cutting_speed, 0.01));
    menu.add_menu_item(std::make_unique<MenuItemBoundedNumber<float>>("Trv Speed", travel_speed, 0.01));
    menu.add_menu_item(std::make_unique<MenuItemBoundedNumber<uint32_t>>("Y Offset", y_offset, 1));
    menu.add_menu_item(std::make_unique<MenuItemMotorControl>(motion_system));
    menu.start_task(joystick);

    homeButton.add_on_press_callback([]()
                                     { motion_system.home(); });

    startButton.add_on_press_callback([]()
                                      { motion_system.move(); });

    stopButton.add_on_press_callback([]()
                                     { motion_system.stop(); });
    ESP_LOGI(TAG, "done setup");
}