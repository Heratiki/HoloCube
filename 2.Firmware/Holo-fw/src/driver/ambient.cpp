/*
 * BH1750FVI Ambient Light Sensor Implementation
 * 
 * This file implements the functionality for the BH1750FVI ambient light sensor:
 * - Sensor initialization and mode configuration
 * - I2C communication
 * - Light measurement and averaging
 */

#include "ambient.h"
#include "common.h"

/**
 * Initialize the ambient light sensor with specified mode
 * Sets up I2C communication and configures sensor operating parameters
 * @param mode Sensor resolution mode affecting sample time and precision
 */
void Ambient::init(int mode)
{
    mMode = mode;
    switch (mode)
    {
    case ONE_TIME_H_RESOLUTION_MODE:
        sample_time = 125;
        break;
    case ONE_TIME_H_RESOLUTION_MODE2:
        sample_time = 125;
        break;
    case ONE_TIME_L_RESOLUTION_MODE:
        sample_time = 20;
        break;
    }

    // Initialize I2C communication
    Wire.begin(AMB_I2C_SDA, AMB_I2C_SCL);

    // Allow time for sensor to stabilize
    delay(50);

    // Configure sensor with specified mode
    Wire.beginTransmission(ADDRESS_BH1750FVI); // Start I2C transmission
    Wire.write(mMode);                         // Set sensor operation mode
    Wire.endTransmission();                    // End transmission
}

/**
 * Read and calculate current ambient light level
 * Performs rolling average of last 5 measurements for stable readings
 * @return Average light level in lux units
 */
unsigned int Ambient::getLux()
{
    // Check if enough time has passed since last reading
    if (millis() - last_time > sample_time)
    {
        last_time = millis();
        
        // Read 2 bytes from sensor (16-bit value)
        Wire.requestFrom(ADDRESS_BH1750FVI, 2);
        highByte = Wire.read();                 // Most significant byte
        lowByte = Wire.read();                  // Least significant byte

        // Convert bytes to illuminance value
        sensorOut = (highByte << 8) | lowByte;  // Combine bytes
        illuminance = sensorOut / 1.2;          // Convert to lux (per datasheet)

        // Update rolling buffer of measurements
        for (int i = 4; i > 0; i--)
            lux[i] = lux[i - 1];
        lux[0] = illuminance;

        // Trigger next measurement
        Wire.beginTransmission(ADDRESS_BH1750FVI);
        Wire.write(mMode);
        Wire.endTransmission();
    }

    // Calculate average of last 5 measurements
    unsigned int avg = 0;
    for (int i = 4; i >= 0; i--)
        avg += lux[i];
    avg /= 5;

    return avg;
}
