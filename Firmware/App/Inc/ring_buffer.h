#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "common_types.h"

/** @brief Buffer capacity in bytes. Must be a power of two for bitwise optimization. */
#define BUFFER_CAPACITY 2048 

/**
 * @brief Circular Buffer structure for raw byte storage.
 * * Uses a lock-free head/tail design where one byte is sacrificed to 
 * distinguish between Full and Empty states without a 'count' variable.
 */
typedef struct {
    uint8_t buffer[BUFFER_CAPACITY];
    uint16_t head;
    uint16_t tail;
} ByteBuffer_t;

/** @brief Magic number to identify the start of a data packet. */
#define PACKET_SYNC_BYTE 0xAA
/** @brief Unique identifier for IMU sensor data packets. */
#define SENSOR_ID_IMU 0x01

/**
 * @brief API Status codes for Buffer operations.
 */
typedef enum {
    BUFFER_OK = 0,                  /**< Operation successful */
    BUFFER_ERR_NULL_PTR,            /**< Null pointer passed as argument */
    BUFFER_ERR_INSUFFICIENT_SPACE,  /**< Not enough space to push data */
    BUFFER_ERR_EMPTY,               /**< Not enough data to pop requested length */
    BUFFER_ERR_INVALID_PARAM        /**< Invalid length or argument provided */
} BufferStatus_t;

/* --- Buffer Management API --- */

/**
 * @brief Initializes the ring buffer indices.
 * @param rb Pointer to the buffer structure.
 * @return BUFFER_OK or BUFFER_ERR_NULL_PTR.
 */
BufferStatus_t Buffer_Init(ByteBuffer_t* rb);

/**
 * @brief Checks if the buffer contains no data.
 * @param rb Pointer to the buffer.
 * @param out_is_empty Pointer to store boolean result.
 * @return BUFFER_OK or error code.
 */
BufferStatus_t Buffer_IsEmpty(const ByteBuffer_t* rb, bool* out_is_empty);

/**
 * @brief Checks if the buffer is full (one byte remaining).
 * @param rb Pointer to the buffer.
 * @param out_is_full Pointer to store boolean result.
 * @return BUFFER_OK or error code.
 */
BufferStatus_t Buffer_IsFull(const ByteBuffer_t* rb, bool* out_is_full);

/**
 * @brief Calculates the number of unread bytes currently in the buffer.
 * @param rb Pointer to the buffer.
 * @param out_count Pointer to store the count.
 * @return BUFFER_OK or error code.
 */
BufferStatus_t Buffer_Get_Count(const ByteBuffer_t* rb, uint16_t* out_count);

/**
 * @brief Calculates available space in the buffer.
 * @param rb Pointer to the buffer.
 * @param out_free_space Pointer to store available bytes.
 * @return BUFFER_OK or error code.
 */
BufferStatus_t Buffer_Get_Free_Space(const ByteBuffer_t* rb, uint16_t* out_free_space);

/**
 * @brief Pushes an array of bytes into the buffer.
 * @param rb Pointer to the buffer.
 * @param data Pointer to the source data array.
 * @param length Number of bytes to push.
 * @return BUFFER_OK, BUFFER_ERR_NULL_PTR, or BUFFER_ERR_INSUFFICIENT_SPACE.
 */
BufferStatus_t Buffer_Push_Array(ByteBuffer_t* rb, const uint8_t* data, uint16_t length);

/**
 * @brief Pops an array of bytes from the buffer.
 * @param rb Pointer to the buffer.
 * @param out_data Pointer to the destination array.
 * @param length Number of bytes to pop.
 * @return BUFFER_OK, BUFFER_ERR_NULL_PTR, or BUFFER_ERR_EMPTY.
 */
BufferStatus_t Buffer_Pop_Array(ByteBuffer_t* rb, uint8_t* out_data, uint16_t length);

/* --- Serialization API --- */

/**
 * @brief Serializes IMU data into a formatted byte packet with checksum.
 * @param imu_data Pointer to the source IMU reading.
 * @param timestamp System tick or timestamp.
 * @param out_packet Pointer to the destination byte array (min 20 bytes).
 * @param out_len Pointer to store the final packet length.
 * @return BUFFER_OK or error code.
 */
BufferStatus_t Serialize_IMU_Data(const IMU_Reading_t* imu_data, uint32_t timestamp, uint8_t* out_packet, uint16_t* out_len);

#endif // RING_BUFFER_H