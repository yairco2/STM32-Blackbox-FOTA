#include "bt_manager.h"
#include <string.h>

/** @brief Maximum allowed length for a single Bluetooth command */
#define MAX_CMD_LENGTH 32

/* --- Internal State --- */
static char rx_buffer[MAX_CMD_LENGTH];
static uint8_t rx_index = 0;

void BT_Init(void) {
	// Clear the memory and reset the index
	memset(rx_buffer, 0, MAX_CMD_LENGTH);
	rx_index = 0;
}

BT_Command_t BT_Process_Byte(char c) {
	// Check for the End Of Command signal (Newline or Carriage Return)
	if (c == '\n' || c == '\r') {
		
		// Ignore empty lines
		if (rx_index == 0) {
			return CMD_NONE;
		}

		// Null-terminate the string
		rx_buffer[rx_index] = '\0';
		
		BT_Command_t parsed_cmd = CMD_UNKNOWN;
		
		// Lookup in the Command Dictionary
		if (strcmp(rx_buffer, "LOG START") == 0) {
			parsed_cmd = CMD_LOG_START;
		} else if (strcmp(rx_buffer, "LOG STOP") == 0) {
			parsed_cmd = CMD_LOG_STOP;
		} else if (strcmp(rx_buffer, "STATUS") == 0) {
			parsed_cmd = CMD_STATUS;
		} else if (strcmp(rx_buffer, "FOTA") == 0) {
			parsed_cmd = CMD_FOTA_BEGIN;
		}
		
		// Reset the buffer so it'll be ready for the next incoming command
		rx_index = 0;
		
		return parsed_cmd;
	}
	
	// Accumulate normal characters
	// We leave 1 byte for Null-Terminator
	if (rx_index < (MAX_CMD_LENGTH - 1)) {
		rx_buffer[rx_index++] = c;
		return CMD_NONE;
	} else {
		// Safety check: Buffer-Overflow prevention, reject if we didn't received newline
		rx_index = 0;
		return CMD_BUFFER_OVERFLOW;
	}
}