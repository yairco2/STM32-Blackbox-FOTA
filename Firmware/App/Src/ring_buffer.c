#include "ring_buffer.h"
#include <stddef.h> // For NULL definition

// ==============================================================================
// 1. Initialization and Lock-Free State Management
// ==============================================================================

BufferStatus_t Buffer_Init(ByteBuffer_t* rb) {
    if (rb == NULL) return BUFFER_ERR_NULL_PTR;
    
    rb->head = 0;
    rb->tail = 0;
    return BUFFER_OK;
}

BufferStatus_t Buffer_IsEmpty(const ByteBuffer_t* rb, bool* out_is_empty) {
    if (rb == NULL || out_is_empty == NULL) return BUFFER_ERR_NULL_PTR;
    
    *out_is_empty = (rb->head == rb->tail);
    return BUFFER_OK;
}

BufferStatus_t Buffer_IsFull(const ByteBuffer_t* rb, bool* out_is_full) {
    if (rb == NULL || out_is_full == NULL) return BUFFER_ERR_NULL_PTR;
    
    uint16_t next_head = (rb->head + 1) & (BUFFER_CAPACITY - 1);
    *out_is_full = (next_head == rb->tail);
    return BUFFER_OK;
}

BufferStatus_t Buffer_Get_Count(const ByteBuffer_t* rb, uint16_t* out_count) {
    if (rb == NULL || out_count == NULL) return BUFFER_ERR_NULL_PTR;
    
    *out_count = (rb->head - rb->tail) & (BUFFER_CAPACITY - 1);
    return BUFFER_OK;
}

BufferStatus_t Buffer_Get_Free_Space(const ByteBuffer_t* rb, uint16_t* out_free_space) {
    if (rb == NULL || out_free_space == NULL) return BUFFER_ERR_NULL_PTR;
    
    uint16_t count = 0;
    Buffer_Get_Count(rb, &count); // Reuse internal logic
    
    *out_free_space = (BUFFER_CAPACITY - 1) - count;
    return BUFFER_OK;
}

// ==============================================================================
// 2. Data Transfer (Push and Pop)
// ==============================================================================

BufferStatus_t Buffer_Push_Array(ByteBuffer_t* rb, const uint8_t* data, uint16_t length) {
    if (rb == NULL || data == NULL) return BUFFER_ERR_NULL_PTR;
    if (length == 0) return BUFFER_ERR_INVALID_PARAM;
    
    uint16_t free_space = 0;
    Buffer_Get_Free_Space(rb, &free_space);
    
    if (free_space < length) return BUFFER_ERR_INSUFFICIENT_SPACE;
    
    for (uint16_t i = 0; i < length; i++) {
        rb->buffer[rb->head] = data[i];
        rb->head = (rb->head + 1) & (BUFFER_CAPACITY - 1);
    }
    
    return BUFFER_OK;
}

BufferStatus_t Buffer_Pop_Array(ByteBuffer_t* rb, uint8_t* out_data, uint16_t length) {
    if (rb == NULL || out_data == NULL) return BUFFER_ERR_NULL_PTR;
    
    uint16_t count = 0;
    Buffer_Get_Count(rb, &count);
    
    if (count < length) return BUFFER_ERR_EMPTY;
    
    for (uint16_t i = 0; i < length; i++) {
        out_data[i] = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) & (BUFFER_CAPACITY - 1);
    }
    
    return BUFFER_OK;
}

// ==============================================================================
// 3. Serialization Protocol (Struct -> Raw Bytes)
// ==============================================================================

BufferStatus_t Serialize_IMU_Data(const IMU_Reading_t* imu_data, uint32_t timestamp, uint8_t* out_packet, uint16_t* out_len) {
    if (imu_data == NULL || out_packet == NULL || out_len == NULL) return BUFFER_ERR_NULL_PTR;
    
    uint16_t idx = 0;
    
    out_packet[idx++] = PACKET_SYNC_BYTE;
    
    // Timestamp (Little-Endian)
    out_packet[idx++] = (uint8_t)(timestamp & 0xFF);
    out_packet[idx++] = (uint8_t)((timestamp >> 8) & 0xFF);
    out_packet[idx++] = (uint8_t)((timestamp >> 16) & 0xFF);
    out_packet[idx++] = (uint8_t)((timestamp >> 24) & 0xFF);

    out_packet[idx++] = SENSOR_ID_IMU;
    out_packet[idx++] = 12; // Payload Length
    
    // IMU Payload (Little-Endian)
    int16_t* channels = (int16_t*)imu_data; // Treat struct as array of int16_t for brevity
    for(int i = 0; i < 6; i++) {
        out_packet[idx++] = (uint8_t)(channels[i] & 0xFF);
        out_packet[idx++] = (uint8_t)((channels[i] >> 8) & 0xFF);
    }
    
    // Checksum
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < idx; i++) {
        checksum ^= out_packet[i];
    }
    out_packet[idx++] = checksum;
    
    *out_len = idx;
    return BUFFER_OK;
}