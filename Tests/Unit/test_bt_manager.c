#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bt_manager.h"

#define RUN_TEST(test) do { \
    printf("Running %s... ", #test); \
    test(); \
    printf("PASS\n"); \
} while (0)

/**
 * @brief Helper function that simulates the STM32 UART hardware.
 * It drips characters into the parser one by one.
 */
BT_Command_t simulate_uart_rx(const char* str) {
	BT_Command_t last_status = CMD_NONE;
	int len = strlen(str);
	
	for (int i = 0; i < len; i++) {
		last_status = BT_Process_Byte(str[i]);
	}
	return last_status;
}

void test_valid_commands() {
	
	// Starting with a clean state
	BT_Init();
	
	// Simulate typing commands with a newline at the end
	assert(simulate_uart_rx("LOG START\n") == CMD_LOG_START);
	assert(simulate_uart_rx("STATUS\n") == CMD_STATUS);
	
	// Simulate Carriage Return commands
	assert(simulate_uart_rx("LOG STOP\r") == CMD_LOG_STOP);
}

void test_unknown_commands() {
	
	// Starting with a clean state
	BT_Init();
	
	// If the user types garbage the system won't crash, just report unknown
	assert(simulate_uart_rx("MAKE ME COFFEE\n") == CMD_UNKNOWN);
}

void test_buffer_overflow_protection() {
	
	// Starting with a clean state
	BT_Init();
	
	// MAX_CMD_LENGTH is 32, we'll send 40 characters W/O a newline
	const char* malicious_string = "THIS_STRING_IS_WAY_TOO_LONG_AND_WILL_CRASH_A_BAD_SYSTEM";
	BT_Command_t status = CMD_NONE;
	
	for (size_t i = 0; i < strlen(malicious_string); i++) {
		status = BT_Process_Byte(malicious_string[i]);
		if (status == CMD_BUFFER_OVERFLOW) {
			break;		// Parser correctly threw its shields up
		}
	}
	
	assert(status == CMD_BUFFER_OVERFLOW);
}

int main() {
    printf("=== Starting Bluetooth CLI Parser Tests ===\n");
    RUN_TEST(test_valid_commands);
    RUN_TEST(test_unknown_commands);
    RUN_TEST(test_buffer_overflow_protection);
    printf("=== All CLI Tests Passed ===\n");
    return 0;
}