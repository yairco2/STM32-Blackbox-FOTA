#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fota_handler.h"

// Helper to calculate the expected CRC for our simulated firmware
uint32_t Calculate_Test_CRC(const uint8_t* data, uint16_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

int main(void) {
    printf("=== Running FOTA Unit Test ===\n");

    // 1. Generate 500 bytes of "Dummy Firmware" (Filled with 0xAA)
    uint32_t fw_size = 500;
    uint8_t dummy_firmware[500];
    memset(dummy_firmware, 0xAA, fw_size);

    // 2. Setup the FOTA Header
    FOTA_Header_t header = {
        .magic_word = FOTA_MAGIC_WORD,
        .version = 101, // v1.0.1
        .firmware_size = fw_size,
        .expected_crc = Calculate_Test_CRC(dummy_firmware, fw_size)
    };

    // 3. Initialize & Start
    FOTA_Init();
    assert(FOTA_Start(&header) == FOTA_OK);
    printf("[PASS] Header accepted, Flash Erased.\n");

    // 4. Send Chunks (Simulating Bluetooth packets)
    assert(FOTA_Process_Chunk(dummy_firmware, 256) == FOTA_OK);
    printf("[PASS] Chunk 1 written (256 bytes).\n");
    
    assert(FOTA_Process_Chunk(&dummy_firmware[256], 244) == FOTA_OK);
    printf("[PASS] Chunk 2 written (244 bytes).\n");

    // 5. Final Verification
    FOTA_State_t final_state = FOTA_Update();
    assert(final_state == FOTA_STATE_SUCCESS);
    printf("[PASS] CRC Match! FOTA State Machine is SUCCESSFUL.\n");

    printf("=== All FOTA Tests Passed! ===\n");
    return 0;
}