/**
 * @file fota_handler.c
 * @brief Firmware Over-The-Air (FOTA) state machine and validation logic.
 * * This module orchestrates the reception, storage, and verification of 
 * new firmware images. It relies on flash_interface.h to abstract the 
 * physical memory operations, making this code 100% hardware-agnostic.
 */

#include "fota_handler.h"
#include "flash_interface.h"
#include <stddef.h> // For NULL

// Internal State variables
static FOTA_State_t current_state = FOTA_STATE_IDLE;
static FOTA_Header_t active_header;
static uint32_t received_bytes = 0;
static uint32_t running_crc = 0xFFFFFFFF;		// Init value for CRC32

/**
 * @brief Calculates a running IEEE 802.3 CRC32.
 * @param data Pointer to the byte array.
 * @param length Number of bytes to process.
 */
static void Update_CRC32(const uint8_t* data, uint16_t length) {
	
	// Safety check
	if (data == NULL || length == 0) {
		return;	
	}
	
	for (uint16_t i = 0; i < length; i++) {
        running_crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (running_crc & 1) {
                running_crc = (running_crc >> 1) ^ 0xEDB88320;	// 0xEDB88320 is the generator polynomial
            } else {
                running_crc >>= 1;
            }
        }
    }
}

void FOTA_Init(void) {
	current_state = FOTA_STATE_IDLE;
	received_bytes = 0;
	running_crc = 0xFFFFFFFF;
	
	// Initialize the abstract flash hardware
	Flash_Init();
}

FOTA_Status_t FOTA_Start(const FOTA_Header_t* header) {
	
	// Safety check: NULL ptr
	if (header == NULL) {
		return FOTA_ERR_STATE;
	}
	
	// Safety check: Only start if IDLE or recovering from an ERROR
	if (current_state != FOTA_STATE_IDLE && current_state != FOTA_STATE_ERROR) {
		return FOTA_ERR_STATE;
	}
	
	// Security Check: Magic Word (Prevents random data from starting an update)
	if (header->magic_word != FOTA_MAGIC_WORD) {
		current_state = FOTA_STATE_ERROR;
		return FOTA_ERR_BAD_MAGIC;
	}
	
	// Sanity check: Firmware size > 0
	if (header->firmware_size == 0) {
		current_state = FOTA_STATE_ERROR;
		return FOTA_ERR_STATE;
	}
	
	// Accept the header and reset trackers
	active_header = *header;
	received_bytes = 0;
	running_crc = 0xFFFFFFFF;
	
	current_state = FOTA_STATE_ERASING;

	// Hardware Action: Erase the target memory slot
	if (!Flash_Erase_Download_Slot()) {
		current_state = FOTA_STATE_ERROR;
		return FOTA_ERR_FILE_IO;		// Hardware failure
	}
	
	current_state = FOTA_STATE_DOWNLOADING;
	return FOTA_OK;
}

FOTA_Status_t FOTA_Process_Chunk(const uint8_t* chunk_data, uint16_t length) {
	
	// Safety check: Null ptr & Zero-length
	if (chunk_data == NULL || length == 0) {
		return FOTA_ERR_STATE;
	}
	
	// State Check: Must be actively downloading
	if (current_state != FOTA_STATE_DOWNLOADING) {
		return FOTA_ERR_STATE;
	}
	
	// Security Check: Chunk size exceeds maximum allowed buffer
	if (length > FOTA_MAX_CHUNK_SIZE) {
		current_state = FOTA_STATE_ERROR;
		return FOTA_ERR_STATE;
	}
	
	// Security Check: Prevent Buffer Overflow Attack
	if (received_bytes + length > active_header.firmware_size) {
		current_state = FOTA_STATE_ERROR;
		return FOTA_ERR_STATE;
	}
	
	// Hardware Action: Write bytes to memory
	if (!Flash_Write_Chunk(received_bytes, chunk_data, length)) {
		current_state = FOTA_STATE_ERROR;
		return FOTA_ERR_FILE_IO;
	}
	
	// Update progress and cryptograph
	received_bytes += length;
	Update_CRC32(chunk_data, length);
	
	return FOTA_OK;
}

FOTA_State_t FOTA_Update(void) {
	// Check if we've received all expected bytes
	if (current_state == FOTA_STATE_DOWNLOADING) {
		if (received_bytes == active_header.firmware_size) {
			
			current_state = FOTA_STATE_VERIFYING;
			
			// Finalize the CRC32 calculation
			running_crc ^= 0xFFFFFFFF;
			
			// Security check: Comparing calculated CRC to expected CRC
			if (running_crc == active_header.expected_crc) {
				current_state = FOTA_STATE_SUCCESS;
			} else {
				current_state = FOTA_STATE_ERROR;
			}
		}
	}
	
	return current_state;
}