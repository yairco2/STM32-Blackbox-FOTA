#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H

#include <common_types.h>

/** * @brief Toggle this to 0 when you are compiling for the actual STM32 board.
 * Set to 1 for PC-based testing and simulation.
 */
#define IMU_SIMULATION_MODE 1

/**
 * @brief API Status codes for the IMU Driver.
 */
typedef enum {
    IMU_OK = 0,             /**< Operation successful */
    IMU_ERR_NOT_FOUND,      /**< Sensor not responding on I2C bus (Check wiring) */
    IMU_ERR_I2C_FAIL,       /**< Communication error during data transfer */
    IMU_ERR_INVALID_ARG     /**< NULL pointer passed to driver */
} IMU_Status_t;

/* --- Public API --- */

/**
 * @brief Initializes the IMU sensor hardware. 
 * If in simulation mode, resets the internal simulation state.
 * @return IMU_OK on success, or error code if the sensor is unreachable.
 */
IMU_Status_t IMU_Init(void);

/**
 * @brief Fetches the latest accelerometer and gyroscope data.
 * @param out_reading Pointer to the struct where data will be stored.
 * @return IMU_OK on success, or error code on communication failure.
 */
IMU_Status_t IMU_Get_Reading(IMU_Reading_t* out_reading);

#endif // IMU_DRIVER_H