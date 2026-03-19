#include <stdio.h>
#include <assert.h>
#include "imu_driver.h"
#include "ring_buffer.h"
#include "bt_manager.h"
#include "logger_task.h"

int main() {
    printf("=== Starting Full System Integration Test ===\n");

    // 1. Setup
    ByteBuffer_t rb;
    Buffer_Init(&rb);
    IMU_Init();
    BT_Init();
    Logger_Init(&rb);

    // 2. Simulate Bluetooth Command: "LOG START\n"
    const char* cmd = "LOG START\n";
    BT_Command_t last_cmd = CMD_NONE;
    for(size_t i = 0; i < 10; i++) {
        last_cmd = BT_Process_Byte(cmd[i]);
    }
    
    if (last_cmd == CMD_LOG_START) {
        printf("INTEGRATION: Received LOG START command!\n");
        Logger_Enable(true);
    }

    // 3. Simulation Loop: Generate data until we trigger a disk write
    printf("INTEGRATION: Generating IMU data...\n");
    IMU_Reading_t reading;
    uint8_t packet[32];
    uint16_t p_len;
    uint32_t ts = 0;

    // Run until the logger has flushed at least once
    int writes_observed = 0;
    for (int i = 0; i < 200; i++) {
        IMU_Get_Reading(&reading);
        Serialize_IMU_Data(&reading, ts++, packet, &p_len);
        Buffer_Push_Array(&rb, packet, p_len);
        
        if (Logger_Update() == LOG_STATE_READY && i > 60) {
            // If we are back in READY after being in WRITING
            writes_observed++;
        }
    }

    Logger_Enable(false);
    printf("INTEGRATION: Test Complete. Check 'blackbox_log.bin' for data.\n");
    
    return 0;
}