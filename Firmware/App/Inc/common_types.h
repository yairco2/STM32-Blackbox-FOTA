#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>

/**
 * @brief Unified IMU Data Structure
 * This is the shared "language" for the driver, the buffer, and the logger.
 */
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} IMU_Reading_t;

#endif // COMMON_TYPES_H