#ifndef FOTA_HANDLER_H
#define FOTA_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#define FOTA_MAGIC_WORD 0xDEADBEEF // Used to verify the header is valid
#define FOTA_MAX_CHUNK_SIZE 256

/**
 * @brief Represents the states of the FOTA state machine.
 */
typedef enum {
    FOTA_STATE_IDLE,
    FOTA_STATE_ERASING,
    FOTA_STATE_DOWNLOADING,
    FOTA_STATE_VERIFYING,
    FOTA_STATE_SUCCESS,
    FOTA_STATE_ERROR
} FOTA_State_t;

/**
 * @brief Status codes returned by FOTA functions.
 */
typedef enum {
    FOTA_OK,
    FOTA_ERR_BAD_MAGIC,
    FOTA_ERR_FILE_IO,
    FOTA_ERR_STATE,
    FOTA_ERR_CRC_MISMATCH
} FOTA_Status_t;

/**
 * @brief The Header struct sent by the PC before sending actual code.
 * Ensures the STM32 knows exactly what to expect.
 */
typedef struct {
	uint32_t magic_word;		// Must be FOTA_MAGIC_WORD
	uint32_t version;			// E.g: 100 == v1.0.0
	uint32_t firmware_size;		// Total bytes of the new .bin file
	uint32_t expected_crc;		// CRC32 of the entire new firmware
} FOTA_Header_t;

/**
 * @brief Initializes the FOTA module to IDLE state.
 */
void FOTA_Init(void);

/**
 * @brief Starts the FOTA process if the header is valid.
 * @param header Pointer to the parsed header struct from the PC.
 * @return FOTA_OK if accepted, FOTA_ERR_BAD_MAGIC if rejected.
 */
FOTA_Status_t FOTA_Start(const FOTA_Header_t* header);

/**
 * @brief Writes a chunk of received firmware bytes to the flash slot.
 * @param chunk_data Pointer to the raw byte array.
 * @param length Number of bytes in this chunk (max 256).
 * @return FOTA_OK if written successfully.
 */
FOTA_Status_t FOTA_Process_Chunk(const uint8_t* chunk_data, uint16_t length);

/**
 * @brief Periodic update function to check download progress and run CRC.
 * @return The current state of the FOTA process.
 */
FOTA_State_t FOTA_Update(void);

#endif // FOTA_HANDLER_H