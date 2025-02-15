/*
 * Display Driver Interface
 * 
 * This header defines the interface for the display driver using LVGL graphics library:
 * - Display initialization and configuration
 * - Display update routine
 * - Backlight control
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

/**
 * Display Controller Class
 * 
 * Manages the TFT display functionality using LVGL graphics library
 * for rendering and display control.
 */
class Display
{
public:
    /**
     * Initialize the display
     * @param rotation Screen rotation (0-3, each representing 90-degree rotation)
     * @param backLight Initial backlight brightness level (0-255)
     */
    void init(uint8_t rotation, uint8_t backLight);

    /**
     * Main display update routine
     * Should be called regularly in the main loop to handle LVGL tasks
     */
    void routine();

    /**
     * Set display backlight brightness
     * @param level Brightness level (0.0-1.0, where 0 is off and 1 is maximum brightness)
     */
    void setBackLight(float level);
};

#endif
