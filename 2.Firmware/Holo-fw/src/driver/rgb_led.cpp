/*
 * RGB LED Driver Implementation
 * 
 * This file implements the RGB LED control functionality:
 * - LED initialization and configuration
 * - Color setting in RGB and HSV modes
 * - Brightness control
 * - Color animation and transitions
 */

#include "rgb_led.h"
#include "common.h"

/**
 * Initialize LED hardware
 * Configures FastLED library with WS2812 LED settings
 */
void Pixel::init()
{
    FastLED.addLeds<WS2812, RGB_LED_PIN, GRB>(rgb_buffers, RGB_LED_NUM);
    FastLED.setBrightness(200);
}

/**
 * Set LED color using RGB values
 * Updates both LEDs with the same color
 */
Pixel &Pixel::setRGB(int r, int g, int b)
{
    rgb_buffers[0] = CRGB(r, g, b);
    rgb_buffers[1] = CRGB(r, g, b);
    FastLED.show();
    return *this;
}

/**
 * Set LED color using HSV values
 * Updates both LEDs with the same color
 */
Pixel &Pixel::setHVS(uint8_t ih, uint8_t is, uint8_t iv)
{
    rgb_buffers[0].setHSV(ih, is, iv);
    rgb_buffers[1].setHSV(ih, is, iv);
    FastLED.show();
    return *this;
}

/**
 * Create rainbow effect
 * Fills LEDs with a gradient between two colors
 */
Pixel &Pixel::fill_rainbow(int min_r, int max_r,
                          int min_g, int max_g,
                          int min_b, int max_b)
{
    fill_gradient(rgb_buffers, 0, CHSV(50, 255, 255), 29, CHSV(150, 255, 255), SHORTEST_HUES);
    FastLED.show();
    return *this;
}

/**
 * Set LED brightness
 * Scales all colors by the brightness factor
 */
Pixel &Pixel::setBrightness(float duty)
{
    duty = constrain(duty, 0, 1);
    FastLED.setBrightness((uint8_t)(255 * duty));
    FastLED.show();
    return *this;
}

// Global variables for LED control
RgbParam g_rgb;                // Current LED settings
hw_timer_t *rgb_timer;         // Hardware timer
RgbRunStatus rgb_status;       // Current LED state
TimerHandle_t xTimer_rgb = NULL; // FreeRTOS timer handle

/**
 * Initialize RGB LED animation thread
 * Sets up timer and initial LED state
 */
void rgb_thread_init(RgbParam *rgb_setting)
{
    set_rgb(rgb_setting);
}

/**
 * Timer callback for HSV mode
 * Updates HSV values for color animation
 */
void led_hsvOnTimer(TimerHandle_t xTimer)
{
    // Update Hue
    rgb_status.current_h += g_rgb.step_h;
    if (rgb_status.current_h >= g_rgb.max_value_h)
    {
        g_rgb.step_h = (-1) * g_rgb.step_h;
        rgb_status.current_h = g_rgb.max_value_h;
    }
    else if (rgb_status.current_h <= g_rgb.min_value_h)
    {
        g_rgb.step_h = (-1) * g_rgb.step_h;
        rgb_status.current_h = g_rgb.min_value_h;
    }

    // Update Saturation
    rgb_status.current_s += g_rgb.step_s;
    if (rgb_status.current_s >= g_rgb.max_value_s)
    {
        g_rgb.step_s = (-1) * g_rgb.step_s;
        rgb_status.current_s = g_rgb.max_value_s;
    }
    else if (rgb_status.current_s <= g_rgb.min_value_s)
    {
        g_rgb.step_s = (-1) * g_rgb.step_s;
        rgb_status.current_s = g_rgb.min_value_s;
    }

    // Update Value/Brightness
    rgb_status.current_v += g_rgb.step_v;
    if (rgb_status.current_v >= g_rgb.max_value_v)
    {
        g_rgb.step_v = (-1) * g_rgb.step_v;
        rgb_status.current_v = g_rgb.max_value_v;
    }
    else if (rgb_status.current_v <= g_rgb.min_value_v)
    {
        g_rgb.step_v = (-1) * g_rgb.step_v;
        rgb_status.current_v = g_rgb.min_value_v;
    }
    
    count_cur_brightness();

    // Update LED state
    rgb.setHVS(rgb_status.current_h,
               rgb_status.current_s,
               rgb_status.current_v)
        .setBrightness(rgb_status.current_brightness);
}

/**
 * Timer callback for RGB mode
 * Updates RGB values for color animation
 */
void led_rgbOnTimer(TimerHandle_t xTimer)
{
    // Cycle through RGB components
    if (0 == rgb_status.pos) // Red component
    {
        rgb_status.current_r += g_rgb.step_r;
        if (rgb_status.current_r >= g_rgb.max_value_r && g_rgb.step_r > 0)
        {
            rgb_status.pos = 1;
            rgb_status.current_r = g_rgb.max_value_r;
        }
        else if (rgb_status.current_r <= g_rgb.min_value_r && g_rgb.step_r < 0)
        {
            g_rgb.step_r = (-1) * g_rgb.step_r;
            rgb_status.current_r = g_rgb.min_value_r;
        }
    }
    else if (1 == rgb_status.pos) // Green component
    {
        rgb_status.current_g += g_rgb.step_r;
        if (rgb_status.current_g >= g_rgb.max_value_g && g_rgb.step_r > 0)
        {
            rgb_status.pos = 2;
            rgb_status.current_g = g_rgb.max_value_g;
        }
        else if (rgb_status.current_g <= g_rgb.min_value_g && g_rgb.step_r < 0)
        {
            rgb_status.pos = 0;
            rgb_status.current_g = g_rgb.min_value_g;
        }
    }
    else if (2 == rgb_status.pos) // Blue component
    {
        rgb_status.current_b += g_rgb.step_r;
        if (rgb_status.current_b >= g_rgb.max_value_b && g_rgb.step_r > 0)
        {
            g_rgb.step_r = (-1) * g_rgb.step_r;
            rgb_status.current_b = g_rgb.max_value_b;
        }
        else if (rgb_status.current_b <= g_rgb.min_value_b && g_rgb.step_r < 0)
        {
            rgb_status.pos = 1;
            rgb_status.current_b = g_rgb.min_value_b;
        }
    }

    count_cur_brightness();

    // Update LED state
    rgb.setRGB(rgb_status.current_r,
               rgb_status.current_g,
               rgb_status.current_b)
        .setBrightness(rgb_status.current_brightness);
}

/**
 * Update brightness level
 * Handles brightness transitions between min and max values
 */
void count_cur_brightness(void)
{
    rgb_status.current_brightness += g_rgb.brightness_step;
    if (rgb_status.current_brightness >= g_rgb.max_brightness)
    {
        rgb_status.current_brightness = g_rgb.max_brightness;
        g_rgb.brightness_step = (-1) * g_rgb.brightness_step;
    }
    else if (rgb_status.current_brightness <= g_rgb.min_brightness)
    {
        rgb_status.current_brightness = g_rgb.min_brightness;
        g_rgb.brightness_step = (-1) * g_rgb.brightness_step;
    }
}

/**
 * Update RGB LED settings
 * Configures LED mode and starts animation timer
 */
void set_rgb(RgbParam *rgb_setting)
{
    g_rgb = *rgb_setting;
    if (NULL != xTimer_rgb)
    {
        xTimerStop(xTimer_rgb, 0);
        xTimer_rgb = NULL;
    }

    // Initialize based on color mode
    if (LED_MODE_RGB == g_rgb.mode)
    {
        rgb_status.current_r = g_rgb.min_value_r;
        rgb_status.current_g = g_rgb.min_value_g;
        rgb_status.current_b = g_rgb.min_value_b;
        rgb_status.current_brightness = g_rgb.min_brightness;
        rgb_status.pos = 0;
        xTimer_rgb = xTimerCreate("rgb controller",
                                 g_rgb.time / portTICK_PERIOD_MS,
                                 pdTRUE, (void *)0, led_rgbOnTimer);
    }
    else if (LED_MODE_HSV == g_rgb.mode)
    {
        rgb_status.current_h = g_rgb.min_value_h;
        rgb_status.current_s = g_rgb.min_value_s;
        rgb_status.current_v = g_rgb.min_value_v;
        rgb_status.current_brightness = g_rgb.min_brightness;
        rgb_status.pos = 0;
        xTimer_rgb = xTimerCreate("rgb controller",
                                 g_rgb.time / portTICK_PERIOD_MS,
                                 pdTRUE, (void *)0, led_hsvOnTimer);
    }
    xTimerStart(xTimer_rgb, 0);
}

/**
 * Clean up RGB LED resources
 * Stops animation timer
 */
void rgb_thread_del(void)
{
    if (NULL != xTimer_rgb)
    {
        xTimerStop(xTimer_rgb, 0);
        xTimer_rgb = NULL;
    }
}
