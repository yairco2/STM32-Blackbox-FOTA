#ifndef BT_MANAGER_H
#define BT_MANAGER_H

#include <stdint.h>

/**
 * @brief Supported system commands received via Bluetooth.
 */
typedef enum {
    CMD_NONE = 0,           /**< No complete command received yet */
    CMD_LOG_START,          /**< Start writing IMU data to SD/PC */
    CMD_LOG_STOP,           /**< Stop writing data */
    CMD_STATUS,             /**< Request system health/status */
    CMD_FOTA_BEGIN,         /**< Put the system into Firmware Update Mode */
    CMD_UNKNOWN,            /**< A complete string was received, but not recognized */
    CMD_BUFFER_OVERFLOW     /**< Safety state: string was too long for memory */
} BT_Command_t;

/**
 * @brief Initializes the Bluetooth command parser state.
 */
void BT_Init(void);


/**
 * @brief Processes a single character received from the Bluetooth UART.
 * @note This function acts as a state machine. It will return CMD_NONE 
 * until a newline ('\n' or '\r') is received.
 * @param c The incoming character.
 * @return The parsed command, or CMD_NONE if still accumulating.
 */
BT_Command_t BT_Process_Byte(char c);


#endif // BT_MANAGER_H
