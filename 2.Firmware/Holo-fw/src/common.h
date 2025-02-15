/*
 * HoloCube Common Definitions
 * 
 * This header file contains all common definitions and configurations for the HoloCube project:
 * - Hardware pin assignments
 * - Screen configurations
 * - System utility configurations
 * - External device drivers
 * - Global object declarations
 */

#ifndef COMMON_H
#define COMMON_H

// Current version of the All-In-One firmware
#define AIO_VERSION "2.2.0"

#include "Arduino.h"
#include "driver/rgb_led.h"
#include "driver/flash_fs.h"
#include "driver/sd_card.h"
#include "driver/display.h"
#include "driver/ambient.h"
#include "driver/imu.h"
#include "network.h"

// MPU6050 Motion Sensor I2C Pins
#define IMU_I2C_SDA 32    // I2C Data pin for MPU6050
#define IMU_I2C_SCL 33    // I2C Clock pin for MPU6050

// Global object declarations for hardware components
extern IMU mpu;           // Motion sensor (MPU6050) interface - only for main program use
extern SdCard tf;         // SD Card interface
extern Pixel rgb;         // RGB LED control
extern Network g_network; // Network connection manager
extern FlashFS g_flashCfg;// Flash filesystem (replaces previous Preferences)
extern Display screen;    // Display interface
extern Ambient ambLight;  // Ambient light sensor interface

boolean doDelayMillisTime(unsigned long interval,
                          unsigned long *previousMillis,
                          boolean state);

// Ambient Light Sensor I2C Pins (shared with MPU6050)
#define AMB_I2C_SDA 32
#define AMB_I2C_SCL 33

// Screen Resolution
#define SCREEN_HOR_RES 240     // Horizontal resolution
#define SCREEN_VER_RES 240     // Vertical resolution

// TFT屏幕接口
// #define PEAK
#ifdef PEAK
#define LCD_BL_PIN 12
/* Battery */
#define CONFIG_BAT_DET_PIN 37
#define CONFIG_BAT_CHG_DET_PIN 38
/* Power */
#define CONFIG_POWER_EN_PIN 21
#define CONFIG_ENCODER_PUSH_PIN 27
#else
#define LCD_BL_PIN 5
#endif

#define LCD_BL_PWM_CHANNEL 0

/**
 * System Utility Configuration Structure
 * Stores various system settings including:
 * - WiFi credentials for up to 3 networks
 * - System behavior settings
 * - Display settings
 * - Motion sensor settings
 */
struct SysUtilConfig
{
    String ssid_0;                // Primary WiFi network SSID
    String password_0;            // Primary WiFi password
    String ssid_1;                // Secondary WiFi SSID
    String password_1;            // Secondary WiFi password
    String ssid_2;                // Tertiary WiFi SSID
    String password_2;            // Tertiary WiFi password
    String auto_start_app;        // Name of app to auto-start on boot
    uint8_t power_mode;           // Power mode (0=eco mode, 1=performance mode)
    uint8_t backLight;            // Screen brightness (1-100)
    uint8_t rotation;             // Screen rotation
    uint8_t auto_calibration_mpu; // MPU auto-calibration (0=off, 1=on)
    uint8_t mpu_order;            // Motion control orientation
};

#define GFX 0

#if GFX
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises
#include <Arduino_GFX_Library.h>
extern Arduino_HWSPI *bus;
extern Arduino_ST7789 *tft;

#else
#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
extern TFT_eSPI *tft;
#endif

#endif
