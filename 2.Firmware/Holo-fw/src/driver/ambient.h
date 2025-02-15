/*
 * BH1750FVI Ambient Light Sensor Driver
 * 
 * This driver provides an interface to the BH1750FVI ambient light sensor:
 * - Supports multiple resolution modes
 * - Provides light measurements in lux
 * - Uses I2C communication protocol
 */

#ifndef AMBIENT_H
#define AMBIENT_H

#include <Wire.h>

// BH1750FVI I2C Address (ADDR pin connected to GND)
#define ADDRESS_BH1750FVI 0x23

// Measurement Modes
#define ONE_TIME_H_RESOLUTION_MODE 0x20   // High Resolution Mode: 1 lux resolution, 120ms measurement time
#define ONE_TIME_H_RESOLUTION_MODE2 0x21  // High Resolution Mode 2: 0.5 lux resolution, 120ms measurement time
#define ONE_TIME_L_RESOLUTION_MODE 0x23   // Low Resolution Mode: 4 lux resolution, 16ms measurement time

/**
 * Ambient Light Sensor Class
 * 
 * Handles communication with the BH1750FVI ambient light sensor and
 * provides methods to initialize the sensor and read light measurements.
 */
class Ambient
{
private:
    int mMode;                     // Current sensor operation mode
    unsigned char highByte = 0;    // High byte of sensor reading
    unsigned char lowByte = 0;     // Low byte of sensor reading
    unsigned int sensorOut = 0;    // Raw sensor output
    unsigned int illuminance = 0;   // Calculated illuminance value

    unsigned int lux[5];           // Buffer for averaging light readings
    long sample_time = 125;        // Time between samples in milliseconds
    long last_time;                // Timestamp of last reading

public:
    /**
     * Initialize the ambient light sensor
     * @param mode Operating mode (ONE_TIME_H_RESOLUTION_MODE, ONE_TIME_H_RESOLUTION_MODE2, or ONE_TIME_L_RESOLUTION_MODE)
     */
    void init(int mode);

    /**
     * Get the current light level in lux
     * @return Current ambient light level in lux units
     */
    unsigned int getLux();
};

#endif
