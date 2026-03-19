#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "ring_buffer.h"

/**
 * @brief The internal states of the Logger State Machine.
 */
typedef enum {
    LOG_STATE_IDLE = 0,     /**< System is standing by. Not logging. */
    LOG_STATE_READY,        /**< Logging is active, waiting for buffer to fill. */
    LOG_STATE_WRITING,      /**< Currently dumping data to the SD card. */
    LOG_STATE_ERROR         /**< SD Card missing, full, or corrupt. */
} Logger_State_t;

/**
 * @brief Status codes for Logger API operations.
 */
typedef enum {
    LOGGER_OK = 0,
    LOGGER_ERR_UNINITIALIZED,
    LOGGER_ERR_SD_FAULT,
    LOGGER_ERR_NULL_PTR
} Logger_Status_t;

/* --- Configuration Macros --- */

/** * @brief The threshold at which the logger decides to write to the SD card.
 * If the buffer capacity is 2048, waiting for 1024 bytes (50%) to fill 
 */
#define LOGGER_WRITE_THRESHOLD (BUFFER_CAPACITY / 2)

/* --- Public API --- */

/**
 * @brief Initializes the Logger task and links it to the data source.
 * @param source_buffer Pointer to the Ring Buffer holding the data.
 * @return LOGGER_OK on success.
 */
Logger_Status_t Logger_Init(ByteBuffer_t* source_buffer);

/**
 * @brief Enables or disables the logging process. 
 * @note This is the function the Bluetooth Manager will call when it receives "LOG START".
 * @param enable True to start logging, False to stop and flush the files.
 */
void Logger_Enable(bool enable);

/**
 * @brief The main state machine tick. 
 * @note This MUST be called continuously inside main Super Loop or FreeRTOS task.
 * It is completely non-blocking.
 * @return The current state of the logger.
 */
Logger_State_t Logger_Update(void);

#endif // LOGGER_TASK_H