/*
 * MPU6050 IMU Implementation
 * 
 * This file implements the functionality for the MPU6050 motion sensor:
 * - Sensor initialization and calibration
 * - Motion detection and classification
 * - Action history management
 * - Virtual motion processing
 */

#include "imu.h"
#include "common.h"

// String descriptions for motion types (used for debugging)
const char *active_type_info[] = {
    "TURN_RIGHT", "RETURN",
    "TURN_LEFT", "UP",
    "DOWN", "GO_FORWORD",
    "SHAKE", "UNKNOWN"
};

/**
 * Constructor
 * Initializes IMU state and action history buffer
 */
IMU::IMU()
{
    // Initialize action info structure
    action_info.isValid = false;
    action_info.active = ACTIVE_TYPE::UNKNOWN;
    action_info.long_time = true;

    // Initialize action history buffer
    for (int pos = 0; pos < ACTION_HISTORY_BUF_LEN; ++pos)
    {
        act_info_history[pos] = UNKNOWN;
    }
    act_info_history_ind = ACTION_HISTORY_BUF_LEN - 1;
    this->order = 0;  // Default orientation
}

/**
 * Initialize the MPU6050 sensor
 * Sets up I2C communication and performs calibration
 * @param order Sensor orientation
 * @param auto_calibration Whether to perform automatic calibration
 * @param mpu_cfg Pointer to calibration configuration
 */
void IMU::init(uint8_t order, uint8_t auto_calibration,
               SysMpuConfig *mpu_cfg)
{
    this->setOrder(order);  // Set sensor orientation
    
    // Initialize I2C communication
    Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
    Wire.setClock(400000);  // Set I2C clock to 400kHz
    
    // Wait for sensor connection
    unsigned long timeout = 5000;
    unsigned long preMillis = millis();
    mpu = MPU6050(0x68);
    while (!mpu.testConnection() && !doDelayMillisTime(timeout, &preMillis, false));

    if (!mpu.testConnection())
    {
        Serial.print(F("Unable to connect to MPU6050.\n"));
        return;
    }

    Serial.print(F("Initialization MPU6050 now, Please don't move.\n"));
    mpu.initialize();

    if (auto_calibration == 0)
    {
        // Use provided calibration values
        mpu.setXGyroOffset(mpu_cfg->x_gyro_offset);
        mpu.setYGyroOffset(mpu_cfg->y_gyro_offset);
        mpu.setZGyroOffset(mpu_cfg->z_gyro_offset);
        mpu.setXAccelOffset(mpu_cfg->x_accel_offset);
        mpu.setYAccelOffset(mpu_cfg->y_accel_offset);
        mpu.setZAccelOffset(mpu_cfg->z_accel_offset);
    }
    else
    {
        // Perform automatic calibration (7 iterations)
        mpu.CalibrateAccel(7);
        mpu.CalibrateGyro(7);
        mpu.PrintActiveOffsets();

        // Store calibration results
        mpu_cfg->x_gyro_offset = mpu.getXGyroOffset();
        mpu_cfg->y_gyro_offset = mpu.getYGyroOffset();
        mpu_cfg->z_gyro_offset = mpu.getZGyroOffset();
        mpu_cfg->x_accel_offset = mpu.getXAccelOffset();
        mpu_cfg->y_accel_offset = mpu.getYAccelOffset();
        mpu_cfg->z_accel_offset = mpu.getZAccelOffset();
    }

    Serial.print(F("Initialization MPU6050 success.\n"));
}

/**
 * Set sensor orientation
 * @param order New orientation configuration
 */
void IMU::setOrder(uint8_t order)
{
    this->order = order;
}

/**
 * Check if encoder button is pressed (Peak hardware specific)
 * @return true if button is pressed
 */
bool IMU::Encoder_GetIsPush(void)
{
#ifdef PEAK
    return (digitalRead(CONFIG_ENCODER_PUSH_PIN) == LOW);
#else
    return false;
#endif
}

/**
 * Update motion detection state
 * Processes sensor data to detect and classify motions
 * @param interval Time between updates in milliseconds
 * @return Pointer to current action information
 */
ImuAction *IMU::update(int interval)
{
    getVirtureMotion6(&action_info);

    if (millis() - last_update_time > interval)
    {
        // Check for rotational motion
        if (!action_info.isValid)
        {
            if (action_info.v_ay > 4000)
            {
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::TURN_LEFT;
            }
            else if (action_info.v_ay < -4000)
            {
                encoder_diff++;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::TURN_RIGHT;
            }
            else if (action_info.v_ay > 1000 || action_info.v_ay < -1000)
            {
                // Detect shaking motion
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        // Check for linear motion
        if (!action_info.isValid)
        {
            if (action_info.v_ax > 5000)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::UP;
                delay(500);
                getVirtureMotion6(&action_info);
                if (action_info.v_ax > 5000)
                {
                    action_info.isValid = 1;
                    action_info.active = ACTIVE_TYPE::GO_FORWORD;
                    encoder_state = LV_INDEV_STATE_PR;
                }
            }
            else if (action_info.v_ax < -5000)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::DOWN;
                delay(500);
                getVirtureMotion6(&action_info);
                if (action_info.v_ax < -5000)
                {
                    action_info.isValid = 1;
                    action_info.active = ACTIVE_TYPE::RETURN;
                    encoder_state = LV_INDEV_STATE_REL;
                }
            }
            else if (action_info.v_ax > 1000 || action_info.v_ax < -1000)
            {
                // Detect shaking motion
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        last_update_time = millis();
    }
    return &action_info;
}

/**
 * Get current motion action
 * Analyzes recent motion data to determine action type
 * Handles both short and long press actions
 * @return Pointer to current action information
 */
ImuAction *IMU::getAction(void)
{
    ImuAction tmp_info;
    getVirtureMotion6(&tmp_info);
    tmp_info.active = ACTIVE_TYPE::UNKNOWN;

    // Check for rotational motion
    if (ACTIVE_TYPE::UNKNOWN == tmp_info.active)
    {
        if (tmp_info.v_ay > 4000)
        {
            tmp_info.active = ACTIVE_TYPE::TURN_LEFT;
        }
        else if (tmp_info.v_ay < -4000)
        {
            tmp_info.active = ACTIVE_TYPE::TURN_RIGHT;
        }
        else if (tmp_info.v_ay > 1000 || tmp_info.v_ay < -1000)
        {
            tmp_info.active = ACTIVE_TYPE::SHAKE;
        }
    }

    // Check for linear motion
    if (ACTIVE_TYPE::UNKNOWN == tmp_info.active)
    {
        if (tmp_info.v_ax > 5000)
        {
            tmp_info.active = ACTIVE_TYPE::UP;
        }
        else if (tmp_info.v_ax < -5000)
        {
            tmp_info.active = ACTIVE_TYPE::DOWN;
        }
        else if (action_info.v_ax > 1000 || action_info.v_ax < -1000)
        {
            tmp_info.active = ACTIVE_TYPE::SHAKE;
        }
    }

    // Update action history buffer
    act_info_history_ind = (act_info_history_ind + 1) % ACTION_HISTORY_BUF_LEN;
    int index = act_info_history_ind;
    act_info_history[index] = tmp_info.active;

    // Process current action sequence
    if (!action_info.isValid)
    {
        bool isHoldDown = false;
        int second = (index + ACTION_HISTORY_BUF_LEN - 1) % ACTION_HISTORY_BUF_LEN;
        int third = (index + ACTION_HISTORY_BUF_LEN - 2) % ACTION_HISTORY_BUF_LEN;

        // Detect short press actions
        if (ACTIVE_TYPE::UNKNOWN != tmp_info.active)
        {
            action_info.isValid = 1;
            action_info.active = tmp_info.active;
        }

        // Detect long press actions
        if (act_info_history[index] == act_info_history[second] && 
            act_info_history[second] == act_info_history[third])
        {
            if (ACTIVE_TYPE::UP == tmp_info.active)
            {
                isHoldDown = true;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::GO_FORWORD;
            }
            else if (ACTIVE_TYPE::DOWN == tmp_info.active)
            {
                isHoldDown = true;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::RETURN;
            }

            if(isHoldDown)
            {
                // Clear history for long press actions
                act_info_history[second] = ACTIVE_TYPE::UNKNOWN;
                act_info_history[third] = ACTIVE_TYPE::UNKNOWN;
            }
        }
    }

    return &action_info;
}

/**
 * Get motion data adjusted for current orientation
 * Reads raw sensor data and applies orientation adjustments
 * @param action_info Pointer to action structure to populate
 */
void IMU::getVirtureMotion6(ImuAction *action_info)
{
    // Read raw motion data
    mpu.getMotion6(&(action_info->v_ax), &(action_info->v_ay),
                   &(action_info->v_az), &(action_info->v_gx),
                   &(action_info->v_gy), &(action_info->v_gz));

    // Apply axis inversions based on orientation
    if (order & X_DIR_TYPE)
    {
        action_info->v_ax = -action_info->v_ax;
        action_info->v_gx = -action_info->v_gx;
    }

    if (order & Y_DIR_TYPE)
    {
        action_info->v_ay = -action_info->v_ay;
        action_info->v_gy = -action_info->v_gy;
    }

    if (order & Z_DIR_TYPE)
    {
        action_info->v_az = -action_info->v_az;
        action_info->v_gz = -action_info->v_gz;
    }

    // Swap X and Y axes if needed
    if (order & XY_DIR_TYPE)
    {
        int16_t swap_tmp;
        swap_tmp = action_info->v_ax;
        action_info->v_ax = action_info->v_ay;
        action_info->v_ay = swap_tmp;
        swap_tmp = action_info->v_gx;
        action_info->v_gx = action_info->v_gy;
        action_info->v_gy = swap_tmp;
    }
}
