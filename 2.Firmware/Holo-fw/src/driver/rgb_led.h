/*
 * RGB LED Driver
 * 
 * This header defines the interface for controlling RGB LEDs:
 * - Supports both RGB and HSV color modes
 * - Configurable color transitions and animations
 * - Brightness control
 * - Timer-based updates
 */

#ifndef RGB_H
#define RGB_H

#include <FastLED.h>
#include <esp32-hal-timer.h>

#define RGB_LED_NUM 2        // Number of RGB LEDs in the chain
#define RGB_LED_PIN 27       // GPIO pin connected to LED data line

// Color modes
#define LED_MODE_RGB 0       // Direct RGB color control
#define LED_MODE_HSV 1       // HSV color space control

/**
 * Pixel Control Class
 * Provides direct control over RGB LEDs
 */
class Pixel
{
private:
    CRGB rgb_buffers[RGB_LED_NUM];  // LED color buffers

public:
    /**
     * Initialize LED hardware
     */
    void init();

    /**
     * Set LED color using RGB values
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @return Reference to this object for method chaining
     */
    Pixel &setRGB(int r, int g, int b);

    /**
     * Set LED color using HSV values
     * @param ih Hue (0-255)
     * @param is Saturation (0-255)
     * @param iv Value/Brightness (0-255)
     * @return Reference to this object for method chaining
     */
    Pixel &setHVS(uint8_t ih, uint8_t is, uint8_t iv);

    /**
     * Create rainbow effect with specified color ranges
     * @param min_r Minimum red value
     * @param max_r Maximum red value
     * @param min_g Minimum green value
     * @param max_g Maximum green value
     * @param min_b Minimum blue value
     * @param max_b Maximum blue value
     * @return Reference to this object for method chaining
     */
    Pixel &fill_rainbow(int min_r, int max_r,
                       int min_g, int max_g,
                       int min_b, int max_b);

    /**
     * Set LED brightness
     * @param duty Brightness level (0.0-1.0)
     * @return Reference to this object for method chaining
     */
    Pixel &setBrightness(float duty);
};

/**
 * RGB LED Configuration Structure
 * Stores LED animation and behavior settings
 */
struct RgbConfig
{
    uint8_t mode;              // Color mode (RGB or HSV)
    uint8_t min_value_0;       // Minimum value for first component (R or H)
    uint8_t min_value_1;       // Minimum value for second component (G or S)
    uint8_t min_value_2;       // Minimum value for third component (B or V)
    uint8_t max_value_0;       // Maximum value for first component
    uint8_t max_value_1;       // Maximum value for second component
    uint8_t max_value_2;       // Maximum value for third component
    int8_t step_0;            // Step size for first component
    int8_t step_1;            // Step size for second component
    int8_t step_2;            // Step size for third component
    float min_brightness;      // Minimum LED brightness
    float max_brightness;      // Maximum LED brightness
    float brightness_step;     // Brightness change step size
    int time;                 // Timer update interval (milliseconds)
};

/**
 * RGB Parameter Structure
 * Provides unified access to RGB and HSV parameters through unions
 */
struct RgbParam
{
    uint8_t mode;              // Color mode (LED_MODE_RGB or LED_MODE_HSV)
    union
    {
        uint8_t min_value_r;
        uint8_t min_value_h;
    };
    union
    {
        uint8_t min_value_g;
        uint8_t min_value_s;
    };
    union
    {
        uint8_t min_value_b;
        uint8_t min_value_v;
    };

    union
    {
        uint8_t max_value_r;
        uint8_t max_value_h;
    };
    union
    {
        uint8_t max_value_g;
        uint8_t max_value_s;
    };
    union
    {
        uint8_t max_value_b;
        uint8_t max_value_v;
    };

    union
    {
        int8_t step_r;
        int8_t step_h;
    };
    union
    {
        int8_t step_g;
        int8_t step_s;
    };
    union
    {
        int8_t step_b;
        int8_t step_v;
    };

    float min_brightness; // 背光的最小亮度
    float max_brightness; // 背光的最大亮度
    float brightness_step;

    int time; // 定时器的时间
};

/**
 * RGB Runtime Status Structure
 * Tracks current LED state during animations
 */
struct RgbRunStatus
{
    union
    {
        uint8_t current_r;
        uint8_t current_h;
    };
    union
    {
        uint8_t current_g;
        uint8_t current_s;
    };
    union
    {
        uint8_t current_b;
        uint8_t current_v;
    };
    uint8_t pos;
    float current_brightness;
};

/**
 * Initialize RGB LED thread and timer
 * @param rgb_setting Initial LED configuration
 */
void rgb_thread_init(RgbParam *rgb_setting);

/**
 * Update RGB LED settings
 * @param rgb_setting New LED configuration
 */
void set_rgb(RgbParam *rgb_setting);

/**
 * Timer callback for RGB mode updates
 * @param xTimer Timer handle
 */
void led_rgbOnTimer(TimerHandle_t xTimer);

/**
 * Timer callback for HSV mode updates
 * @param xTimer Timer handle
 */
void led_hsvOnTimer(TimerHandle_t xTimer);

/**
 * Calculate current brightness level
 * Updates brightness based on animation settings
 */
void count_cur_brightness(void);

/**
 * Clean up RGB LED resources
 * Stops timer and frees memory
 */
void rgb_thread_del(void);

#endif
