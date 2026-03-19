#include "logger_task.h"
#include <stdio.h>
#include <stddef.h>

/* Simulation vs Hardware Switches */
#define LOGGER_SIMULATION_MODE 1

/* Internal State Management */
static Logger_State_t current_state = LOG_STATE_IDLE;
static ByteBuffer_t* data_source = NULL;
static bool logging_enabled = false;

/* PC Simulation File Handle */
#if LOGGER_SIMULATION_MODE
	static FILE* pc_log_file = NULL;
#endif

Logger_Status_t Logger_Init(ByteBuffer_t* source_buffer) {
	
	// Safety check - NULL PTR
	if (source_buffer == NULL) return LOGGER_ERR_NULL_PTR;
	
	data_source = source_buffer;
	current_state = LOG_STATE_IDLE;
	logging_enabled = false;
	
	return LOGGER_OK;
}

void Logger_Enable(bool enable) {
	logging_enabled = enable;
	
	// Starting: Transition from IDLE to ACTIVE
	if (logging_enabled && current_state == LOG_STATE_IDLE) {
		current_state = LOG_STATE_READY;
		
		#if LOGGER_SIMULATION_MODE
			pc_log_file = fopen("blackbox_log.bin", "ab");	// Open for Append Binary
			if (pc_log_file) printf("LOGGER: File opened for simulation.\n");
		#endif
	} 
	// STOPPING: Return to IDLE and clean up 
	else if (!logging_enabled) {
		current_state = LOG_STATE_IDLE;
		
		#if LOGGER_SIMULATION_MODE
			if (pc_log_file) {
				fclose(pc_log_file);	// Close the file
				pc_log_file = NULL;
				printf("LOGGER: File closed.\n");
			}
		#endif
	}
}

Logger_State_t Logger_Update(void) {
	
	if (data_source == NULL) return LOG_STATE_ERROR;
	if (!logging_enabled) return LOG_STATE_IDLE;
	
	uint16_t current_count = 0;
	Buffer_Get_Count(data_source, &current_count);
	
	// STATE: READY - Waiting for buffer to reach the threshold
	if (current_state == LOG_STATE_READY) {
		if (current_count >= LOGGER_WRITE_THRESHOLD) {
			current_state = LOG_STATE_WRITING;
		}
	}
	
	// STATE: WRITING - Emptying half the buffer to storage
	if (current_state == LOG_STATE_WRITING) {
		uint8_t write_buffer[LOGGER_WRITE_THRESHOLD];
		
		// Pull the data out of our Ring Buffer
		if (Buffer_Pop_Array(data_source, write_buffer, LOGGER_WRITE_THRESHOLD) == BUFFER_OK) {
			
			#if LOGGER_SIMULATION_MODE
				if (pc_log_file) {
					fwrite(write_buffer, 1, LOGGER_WRITE_THRESHOLD, pc_log_file);
					fflush(pc_log_file);	// Ensure it's saved to disk
					printf("LOGGER: Flushed %d bytes to disk.\n", LOGGER_WRITE_THRESHOLD);
				}
			#else
				// FUTURE: f_write(&SDFile, write_buffer, LOGGER_WRITE_THRESHOLD, &bytesWritten);
			#endif
			
			current_state = LOG_STATE_READY;		// Back to waiting
		} else {
			current_state = LOG_STATE_ERROR;
		}
	}
	
	return current_state;
	
}