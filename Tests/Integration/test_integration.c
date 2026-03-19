#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "imu_driver.h"
#include "ring_buffer.h"

#define RUN_TEST(test) do { \
    printf("Running %s... ", #test); \
    test(); \
    printf("PASS\n"); \
} while (0)
	
void test_imu_to_buffer_pipeline() {
	ByteBuffer_t rb;
	IMU_Reading_t reading;
	
	// Init both systems
	assert(Buffer_Init(&rb) == BUFFER_OK);
	assert(IMU_Init() == IMU_OK);
	
	// Variables for serialization
	uint8_t packet[32];					// Max packet size
	uint16_t packet_len = 0;			
	uint32_t fake_timestamp = 1000;		// 1 second
	
	// Read from IMU
	assert(IMU_Get_Reading(&reading) == IMU_OK);
	
	// Serialize the data
	assert(Serialize_IMU_Data(&reading, fake_timestamp, packet, &packet_len) == BUFFER_OK);
	
	// Quick math check: Sync(1) + Time(4) + ID(1) + Len(1) + Payload(12) + Checksum(1)= 20 bytes
	assert(packet_len == 20);
	assert(packet[0] == PACKET_SYNC_BYTE);		// Verify sync byte
	
	// Push the serialized packet into the ring buffer
	assert(Buffer_Push_Array(&rb, packet, packet_len) == BUFFER_OK);
	
	// Verify the buffer state
	uint16_t count;
	assert(Buffer_Get_Count(&rb, &count) == BUFFER_OK);
	assert(count == 20);
	
	// Pop and verify the Checksum
	uint8_t popped_packet[32];
	assert(Buffer_Pop_Array(&rb, popped_packet, packet_len) == BUFFER_OK);
	
	// Verify the popped data matches what we pushed
	assert(memcmp(packet, popped_packet, packet_len) == 0);	
}

int main() {
    printf("=== Starting System Integration Tests ===\n");
    RUN_TEST(test_imu_to_buffer_pipeline);
    printf("=== All Integration Tests Passed ===\n");
    return 0;
}