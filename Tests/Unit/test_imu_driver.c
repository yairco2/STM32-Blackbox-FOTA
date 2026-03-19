#include <stdio.h>
#include <assert.h>
#include "imu_driver.h"

#define RUN_TEST(test) do { \
    printf("Running %s... ", #test); \
    test(); \
    printf("PASS\n"); \
} while (0)
	
void test_imu_null_ptr() {
	assert(IMU_Get_Reading(NULL) == IMU_ERR_INVALID_ARG);
}

void test_imu_sawtooth_generation() {
	IMU_Reading_t reading;
	
	// Ensure we start from a clean state
	assert(IMU_Init() == IMU_OK);
	
	// Read 1: Counter should be 0
	assert(IMU_Get_Reading(&reading) == IMU_OK);
	assert(reading.accel_x == 0);
	assert(reading.accel_y == 100);
	assert(reading.accel_z == 200);
	
	// Read 2: Counter should have incremented by 10
	assert(IMU_Get_Reading(&reading) == IMU_OK);
	assert(reading.accel_x == 10);
	assert(reading.accel_y == 110);
	assert(reading.accel_z == 210);
}

int main() {
    printf("=== Starting IMU Driver Unit Tests ===\n");
    RUN_TEST(test_imu_null_ptr);
    RUN_TEST(test_imu_sawtooth_generation);
    printf("=== All IMU Tests Passed ===\n");
    return 0;
}