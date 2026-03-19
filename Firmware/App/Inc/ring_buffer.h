#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

// Define the IMU Data Payload
typedef struct {
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
} IMU_Reading_t;

// Define Buffer's capacity (raw bytes)
#define BUFFER_CAPACITY 2048 // 2KB of raw memory

// Define the buffer itself
typedef struct {
	uint8_t buffer[BUFFER_CAPACITY];
	uint16_t head;
	uint16_t tail;
} ByteBuffer_t;


// Protocol Magic Numbers
#define PACKET_SYNC_BYTE 0xAA
#define SENSOR_ID_IMU 0x01


/* --- API Status Code --- */
typedef enum {
    BUFFER_OK = 0,
    BUFFER_ERR_NULL_PTR,
    BUFFER_ERR_INSUFFICIENT_SPACE,
    BUFFER_ERR_EMPTY,
    BUFFER_ERR_INVALID_PARAM
} BufferStatus_t;


/* 	--- Buffer Management API --- */


// Initialization
BufferStatus_t Buffer_Init(ByteBuffer_t* rb);

// State Check API - Data is returned via out-pointers
BufferStatus_t Buffer_IsEmpty(const ByteBuffer_t* rb, bool* out_is_empty);
BufferStatus_t Buffer_IsFull(const ByteBuffer_t* rb, bool* out_is_full);
BufferStatus_t Buffer_Get_Count(const ByteBuffer_t* rb, uint16_t* out_count);
BufferStatus_t Buffer_Get_Free_Space(const ByteBuffer_t* rb, uint16_t* out_free_space);

// Data Management API
BufferStatus_t Buffer_Push_Array(ByteBuffer_t* rb, const uint8_t* data, uint16_t length);
BufferStatus_t Buffer_Pop_Array(ByteBuffer_t* rb, uint8_t* out_data, uint16_t length);

/* --- Serialization API --- */
// Returns length via out-pointer to keep signature consistent
BufferStatus_t Serialize_IMU_Data(const IMU_Reading_t* imu_data, uint32_t timestamp, uint8_t* out_packet, uint16_t* out_len);



#endif // RING_BUFFER_H



