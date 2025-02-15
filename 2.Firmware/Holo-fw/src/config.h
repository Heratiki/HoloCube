/*
 * HoloCube Configuration System
 * 
 * This header file defines the configuration structures and functions for:
 * - MPU6050 calibration data
 * - Network settings
 * - Weather service settings
 * - Display preferences
 * - Motion sensor settings
 */

#ifndef CONFIG_H
#define CONFIG_H
#include <WString.h>
#include <Preferences.h>

extern Preferences prefs;       // Preferences object for persistent storage

/**
 * MPU6050 Motion Sensor Calibration Configuration
 * Stores calibration offsets for both gyroscope and accelerometer
 */
struct MPU_Config
{
    int16_t x_gyro_offset;     // X-axis gyroscope calibration offset
    int16_t y_gyro_offset;     // Y-axis gyroscope calibration offset
    int16_t z_gyro_offset;     // Z-axis gyroscope calibration offset

    int16_t x_accel_offset;    // X-axis accelerometer calibration offset
    int16_t y_accel_offset;    // Y-axis accelerometer calibration offset
    int16_t z_accel_offset;    // Z-axis accelerometer calibration offset
};

/**
 * Main Configuration Structure
 * Stores all device settings including:
 * - Network credentials
 * - Weather service configuration
 * - Display settings
 * - Motion sensor settings
 */
struct Config
{
    // Network Settings
    String ssid;                  // WiFi network SSID
    String password;              // WiFi network password
    
    // Weather Service Settings
    String cityname;              // City name for weather display
    String language;              // Language code for weather queries
    String weather_key;           // Weather API key
    String tianqi_appid;          // TianQi API ID
    String tianqi_appsecret;      // TianQi API secret
    String tianqi_addr;           // TianQi location (in Chinese)
    
    // Social Media Settings
    String bili_uid;              // Bilibili user ID
    
    // Display Settings
    uint8_t backLight;            // Screen brightness (1-100)
    uint8_t rotation;             // Screen rotation direction
    
    // Motion Sensor Settings
    uint8_t auto_calibration_mpu; // Auto-calibration flag (0=off, 1=on)
    uint8_t mpu_order;            // Motion control orientation
    MPU_Config mpu_config;        // MPU calibration data
};

/**
 * Read configuration from storage
 * @param file_path Path to configuration file
 * @param cfg Pointer to Config structure to populate
 */
void config_read(const char *file_path, Config *cfg);

/**
 * Save configuration to storage
 * @param file_path Path to configuration file
 * @param cfg Pointer to Config structure to save
 */
void config_save(const char *file_path, Config *cfg);

/**
 * Read MPU calibration data from storage
 * @param file_path Path to MPU configuration file
 * @param cfg Pointer to Config structure to populate with MPU data
 */
void mpu_config_read(const char *file_path, Config *cfg);

/**
 * Save MPU calibration data to storage
 * @param file_path Path to MPU configuration file
 * @param cfg Pointer to Config structure containing MPU data to save
 */
void mpu_config_save(const char *file_path, Config *cfg);

#endif
