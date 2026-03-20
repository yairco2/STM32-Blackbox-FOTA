#include "flash_interface.h"
#include <stdio.h>

#define SIM_FLASH_FILE "simulated_flash_slot_1.bin"
static FILE* flash_file = NULL;

bool Flash_Init(void) {
    return true; // Nothing to init on PC
}

bool Flash_Erase_Download_Slot(void) {
    // "wb" mode automatically truncates (erases) the file to 0 bytes
    flash_file = fopen(SIM_FLASH_FILE, "wb");
    if (flash_file == NULL) return false;
    
    fclose(flash_file);
    return true;
}

bool Flash_Write_Chunk(uint32_t offset, const uint8_t* data, uint16_t length) {
    // Open for appending binary
    flash_file = fopen(SIM_FLASH_FILE, "ab");
    if (flash_file == NULL) return false;
    
    // PC append simulation
    size_t written = fwrite(data, 1, length, flash_file);
    fclose(flash_file);
    
    return (written == length);
}