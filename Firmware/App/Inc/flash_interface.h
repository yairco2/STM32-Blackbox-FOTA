#ifndef FLASH_INTERFACE_H
#define FLASH_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes the flash subsystem.
 */
bool Flash_Init(void);

/**
 * @brief Erases the entire Download Slot (Slot 1).
 */
bool Flash_Erase_Download_Slot(void);

/**
 * @brief Writes a chunk of data to the Download Slot.
 */
bool Flash_Write_Chunk(uint32_t offset, const uint8_t* data, uint16_t length);

#endif // FLASH_INTERFACE_H