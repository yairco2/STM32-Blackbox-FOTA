#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ring_buffer.h"

// Macro for output
#define RUN_TEST(test) do { \
	printf("Running %s... ", #test); \
	test(); \
	printf("PASS\n");\
} while (0)
	
void test_init_null() {
	assert(Buffer_Init(NULL) == BUFFER_ERR_NULL_PTR);
}

void test_push_pop_basic() {
	ByteBuffer_t rb;
	Buffer_Init(&rb);
	
	uint8_t data_in[] = {0xDE, 0xAD, 0xBE, 0xEF};
	uint8_t data_out[4] = {0};
	
	assert(Buffer_Push_Array(&rb, data_in, 4) == BUFFER_OK);
	
	uint16_t count;
	Buffer_Get_Count(&rb, &count);
	assert(count == 4);
	
	assert(Buffer_Pop_Array(&rb, data_out, 4) == BUFFER_OK);
	assert(memcmp(data_in, data_out, 4) == 0);
}

void test_buffer_wrap_around() {
    ByteBuffer_t rb;
    Buffer_Init(&rb);
    
    uint8_t dummy_byte = 0xAA;
    bool is_full = false;
    uint16_t count = 0;

    // 1. Fill the buffer exactly to its limit (CAPACITY - 1)
    for (int i = 0; i < (BUFFER_CAPACITY - 1); i++) {
        assert(Buffer_Push_Array(&rb, &dummy_byte, 1) == BUFFER_OK);
    }

    // 2. Verify it is full using the API
    assert(Buffer_IsFull(&rb, &is_full) == BUFFER_OK);
    assert(is_full == true);
    
    // 3. Verify that pushing even 1 more byte fails
    assert(Buffer_Push_Array(&rb, &dummy_byte, 1) == BUFFER_ERR_INSUFFICIENT_SPACE);

    // 4. Pop exactly half of the buffer to move the 'tail' pointer
    uint16_t pop_size = BUFFER_CAPACITY / 2;
    uint8_t output_trash[BUFFER_CAPACITY]; // Large enough for any test pop
    assert(Buffer_Pop_Array(&rb, output_trash, pop_size) == BUFFER_OK);

    // 5. Push data back in. 
    // Because we popped from the front, this push will force 'head' 
    // to wrap around the end of the array back to index 0.
    assert(Buffer_Push_Array(&rb, output_trash, pop_size) == BUFFER_OK);

    // 6. Final verification of count
    assert(Buffer_Get_Count(&rb, &count) == BUFFER_OK);
    assert(count == (BUFFER_CAPACITY - 1));
}

int main() {
    printf("=== Starting Ring Buffer Unit Tests ===\n");
    
    RUN_TEST(test_init_null);
    RUN_TEST(test_push_pop_basic);
    RUN_TEST(test_buffer_wrap_around);
    
    printf("=== All Tests Passed Successfully ===\n");
    return 0;
}