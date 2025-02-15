/*
 * MPU6050 IMU (Inertial Measurement Unit) Driver
 * 
 * This header defines the interface for the MPU6050 motion sensor:
 * - Motion detection and classification
 * - Sensor calibration
 * - Action history tracking
 * - Virtual motion processing
 */

#ifndef IMU_H
#define IMU_H

#include <I2Cdev.h>
#include <MPU6050.h>
#include "lv_port_indev.h"
#include <list>

#define ACTION_HISTORY_BUF_LEN 5  // Length of action history buffer

extern int32_t encoder_diff;
extern lv_indev_state_t encoder_state;

extern const char *active_type_info[];

/**
 * Motion Action Types
 * Defines possible motion actions that can be detected
 */
enum ACTIVE_TYPE
{
    TURN_RIGHT = 0,  // Clockwise rotation
    RETURN,          // Return to center/neutral position
    TURN_LEFT,       // Counter-clockwise rotation
    UP,              // Upward motion
    DOWN,            // Downward motion
    GO_FORWORD,      // Forward motion
    SHAKE,           // Shaking motion
    UNKNOWN          // Unrecognized motion
};

/**
 * MPU Direction Types
 * Defines axis configurations for motion detection
 */
enum MPU_DIR_TYPE
{
    NORMAL_DIR_TYPE = 0,    // Normal orientation
    X_DIR_TYPE = 0x01,      // X-axis motion
    Y_DIR_TYPE = 0x02,      // Y-axis motion
    Z_DIR_TYPE = 0x04,      // Z-axis motion
    XY_DIR_TYPE = 0x08      // Combined X-Y motion
};

/**
 * MPU Calibration Configuration
 * Stores calibration offsets for gyroscope and accelerometer
 */
struct SysMpuConfig
{
    int16_t x_gyro_offset;    // X-axis gyroscope offset
    int16_t y_gyro_offset;    // Y-axis gyroscope offset
    int16_t z_gyro_offset;    // Z-axis gyroscope offset

    int16_t x_accel_offset;   // X-axis accelerometer offset
    int16_t y_accel_offset;   // Y-axis accelerometer offset
    int16_t z_accel_offset;   // Z-axis accelerometer offset
};

/**
 * IMU Action Data Structure
 * Contains current motion state and sensor readings
 */
struct ImuAction
{
    volatile ACTIVE_TYPE active;  // Current detected motion type
    boolean isValid;             // Whether the action is valid
    boolean long_time;           // Whether action has been sustained
    int16_t v_ax;               // Virtual X-axis acceleration (adjusted for initial orientation)
    int16_t v_ay;               // Virtual Y-axis acceleration
    int16_t v_az;               // Virtual Z-axis acceleration
    int16_t v_gx;               // Virtual X-axis gyroscope
    int16_t v_gy;               // Virtual Y-axis gyroscope
    int16_t v_gz;               // Virtual Z-axis gyroscope
};

/**
 * IMU Controller Class
 * Manages the MPU6050 sensor and motion detection
 */
class IMU
{
private:
    MPU6050 mpu;              // MPU6050 sensor instance
    int flag;                 // Status flag
    long last_update_time;    // Last sensor update timestamp
    uint8_t order;           // Orientation order (whether to swap X and Y axes)

public:
    ImuAction action_info;    // Current action information
    ACTIVE_TYPE act_info_history[ACTION_HISTORY_BUF_LEN];  // Action history buffer
    int act_info_history_ind; // Current position in history buffer

public:
    IMU();  // Constructor

    /**
     * Initialize the IMU
     * @param order Axis orientation order
     * @param auto_calibration Enable auto-calibration
     * @param mpu_cfg Pointer to calibration configuration
     */
    void init(uint8_t order, uint8_t auto_calibration, SysMpuConfig *mpu_cfg);

    /**
     * Set the sensor orientation
     * @param order New orientation order
     */
    void setOrder(uint8_t order);

    /**
     * Check if encoder button is pressed (for Peak encoder compatibility)
     * @return true if button is pressed
     */
    bool Encoder_GetIsPush(void);

    /**
     * Update sensor readings and motion detection
     * @param interval Update interval in milliseconds
     * @return Pointer to current action information
     */
    ImuAction *update(int interval);

    /**
     * Get current action information
     * @return Pointer to current action information
     */
    ImuAction *getAction(void);

    /**
     * Get virtual motion data (adjusted for orientation)
     * @param action_info Pointer to action structure to populate
     */
    void getVirtureMotion6(ImuAction *action_info);
};

#endif
