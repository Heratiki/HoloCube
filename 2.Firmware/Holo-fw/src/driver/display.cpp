/*
 * Display Driver Implementation
 * 
 * This file implements the display functionality using LVGL and ST7789 TFT:
 * - Display buffer and driver configuration
 * - Screen flushing and rendering
 * - Backlight PWM control
 */

#include "display.h"
#include "network.h"
#include "lv_port_indev.h"
#include "lv_demo_encoder.h"
#include "common.h"

// Maximum number of horizontal lines to buffer
#define LV_HOR_RES_MAX_LEN 80

// LVGL display buffers and driver
static lv_disp_draw_buf_t disp_buf;          // Display buffer for LVGL
static lv_disp_drv_t disp_drv;               // Display driver for LVGL
static lv_color_t buf[SCREEN_HOR_RES * LV_HOR_RES_MAX_LEN];  // Pixel buffer

/**
 * Debug print callback for LVGL
 * Outputs debug messages to Serial port
 */
void my_print(const char * buf)
{
    Serial.printf("%s", buf);
    Serial.flush();
}

/**
 * Display flush callback for LVGL
 * Handles the actual transfer of rendered content to the display
 * @param disp Display driver instance
 * @param area Area to update
 * @param color_p Color buffer containing pixel data
 */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    // Calculate dimensions of area to update
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft->setAddrWindow(area->x1, area->y1, w, h);
    tft->startWrite();
    // tft->writePixels(&color_p->full, w * h);
    tft->pushColors(&color_p->full, w * h, true);
    tft->endWrite();
    // Initiate DMA - blocking only if last DMA is not complete
    // tft->pushImageDMA(area->x1, area->y1, w, h, bitmap, &color_p->full);

    lv_disp_flush_ready(disp);
}

/**
 * Initialize the display hardware and LVGL
 * @param rotation Screen rotation setting
 * @param backLight Initial backlight level (0-100)
 */
void Display::init(uint8_t rotation, uint8_t backLight)
{
    // Setup backlight PWM control
    ledcSetup(LCD_BL_PWM_CHANNEL, 5000, 8);  // 5kHz PWM, 8-bit resolution
    ledcAttachPin(LCD_BL_PIN, LCD_BL_PWM_CHANNEL);

    // Initialize LVGL
    lv_init();

#if LV_USE_LOG
    lv_log_register_print_cb(my_print);  // Register debug print function
#endif

    // Start with backlight off to hide initialization artifacts
    setBackLight(0.0);

    // Initialize TFT display
    tft->begin();
    tft->fillScreen(TFT_BLACK);
    tft->writecommand(ST7789_DISPON);  // Enable display

    /* 
     * Set display rotation:
     * 0 = Normal orientation
     * 4 = Mirror image (for beam splitter)
     * 5 = Side display mode
     */
    tft->setRotation(rotation);

    // Set initial backlight level
    setBackLight(backLight / 100.0);

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, SCREEN_HOR_RES * LV_HOR_RES_MAX_LEN);

    /*Initialize the display*/
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_HOR_RES;
    disp_drv.ver_res = SCREEN_VER_RES;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = tft;
    // 开启 LV_COLOR_SCREEN_TRANSP 屏幕具有透明和不透明样式
    lv_disp_drv_register(&disp_drv);
}

/**
 * Main display update routine
 * Processes LVGL tasks and updates the display
 */
void Display::routine()
{
    lv_task_handler();  // Handle pending LVGL tasks
}

/**
 * Set display backlight level
 * @param duty Brightness level (0.0-1.0)
 * Note: The duty cycle is inverted because the backlight is active LOW
 */
void Display::setBackLight(float duty)
{
    duty = constrain(duty, 0, 1);     // Clamp value between 0 and 1
    duty = 1 - duty;                  // Invert duty cycle (active LOW)
    ledcWrite(LCD_BL_PWM_CHANNEL, (int)(duty * 255));  // Set PWM duty cycle
}
