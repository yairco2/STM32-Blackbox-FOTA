#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "logger_task.h"
#include "ring_buffer.h"

#define RUN_TEST(test) do { \
    printf("Running %s... ", #test); \
    test(); \
    printf("PASS\n"); \
} while (0)

void test_logger_state_transitions() {
    ByteBuffer_t rb;
    Buffer_Init(&rb);
    
    // 1. Initial State should be IDLE
    assert(Logger_Init(&rb) == LOGGER_OK);
    assert(Logger_Update() == LOG_STATE_IDLE);

    // 2. Enable Logging -> Should move to READY
    Logger_Enable(true);
    assert(Logger_Update() == LOG_STATE_READY);

    // 3. Fill buffer slightly (below threshold) -> Should stay READY
    uint8_t dummy_data[10] = {0};
    Buffer_Push_Array(&rb, dummy_data, 10);
    assert(Logger_Update() == LOG_STATE_READY);

    // 4. Fill buffer above threshold -> Should trigger WRITING then back to READY
    // We need 1024 bytes to trigger.
    uint8_t large_chunk[LOGGER_WRITE_THRESHOLD];
    memset(large_chunk, 0xAA, LOGGER_WRITE_THRESHOLD);
    Buffer_Push_Array(&rb, large_chunk, LOGGER_WRITE_THRESHOLD);
    
    // First update triggers the write
    assert(Logger_Update() == LOG_STATE_READY); 
    
    // 5. Disable logging -> Should return to IDLE
    Logger_Enable(false);
    assert(Logger_Update() == LOG_STATE_IDLE);
}

int main() {
    printf("=== Starting Logger Unit Tests ===\n");
    RUN_TEST(test_logger_state_transitions);
    printf("=== All Logger Unit Tests Passed ===\n");
    return 0;
}