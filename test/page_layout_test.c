#include "page_layout.h"
#include <assert.h>

/* Global buffer to act as our in-memory database page */
uint8_t test_frame[DB_PAGE_SIZE];

/* Dummy tuple data for testing */
uint8_t tuple_a[] = "TUPLE_A"; /* 8 bytes including \0 */
uint8_t tuple_b[] = "TUPLE_B"; /* 8 bytes including \0 */
uint8_t tuple_c[] = "TUPLE_C"; /* 8 bytes including \0 */
uint8_t tuple_mod[] = "MOD_TUP"; /* 8 bytes including \0 */

void test_pl_frame_create_success(void) {
    /* Assuming pl_frame_create was updated to take the buffer as an argument */
    pl_frame_create(test_frame);
    
    assert(pl_frame_get_slot_count(test_frame) == 0);
    assert(pl_frame_get_free_space_pointer(test_frame) == DB_PAGE_SIZE);
    assert(pl_frame_get_fragmented_space(test_frame) == 0);
}

void test_pl_frame_insert_success(void) {
    uint16_t start_offset;

    /* Insert first tuple */
    assert(pl_frame_insert_slot(test_frame, tuple_a, 8) == 0);
    assert(pl_frame_get_slot_count(test_frame) == 1);
    assert(pl_frame_get_free_space_pointer(test_frame) == DB_PAGE_SIZE - 8);
    
    /* Insert second tuple */
    assert(pl_frame_insert_slot(test_frame, tuple_b, 8) == 0);
    assert(pl_frame_get_slot_count(test_frame) == 2);
    assert(pl_frame_get_free_space_pointer(test_frame) == DB_PAGE_SIZE - 16);

    /* Verify the data was actually written to the correct location */
    start_offset = pl_frame_slot_get_start(test_frame, 0);
    assert(memcmp(test_frame + start_offset, tuple_a, 8) == 0);
}

void test_pl_frame_insert_out_of_bounds_error(void) {
    /* Create a massive dummy tuple that exceeds the page size */
    uint16_t massive_size = DB_PAGE_SIZE + 1;
    uint8_t massive_tuple[1]; /* Content doesn't matter, just the size parameter */
    
    assert(pl_frame_insert_slot(test_frame, massive_tuple, massive_size) == 1);
}

void test_pl_frame_update_success(void) {
    uint16_t start_offset;

    /* Update slot 0 with new data of the EXACT same size */
    assert(pl_frame_update_slot(test_frame, 0, tuple_mod, 8) == 0);
    
    /* Verify the data changed */
    start_offset = pl_frame_slot_get_start(test_frame, 0);
    assert(memcmp(test_frame + start_offset, tuple_mod, 8) == 0);
}

void test_pl_frame_update_size_mismatch_error(void) {
    /* Attempt to update slot 1 with a size that does not match its original size */
    assert(pl_frame_update_slot(test_frame, 1, tuple_mod, 99) == 1);
}

void test_pl_frame_delete_success(void) {
    uint16_t initial_fragmentation = pl_frame_get_fragmented_space(test_frame);
    
    /* Delete slot 0. Note: Your code currently returns 1 at the end of delete! */
    assert(pl_frame_delete_slot(test_frame, 0) == 1); 
    
    /* Verify the slot points to 0 and fragmentation increased */
    assert(pl_frame_slot_get_start(test_frame, 0) == 0);
    assert(pl_frame_get_fragmented_space(test_frame) == initial_fragmentation + 8);
}

void test_pl_frame_compact_success(void) {
    uint16_t new_start;
    /* Before compaction, we have a deleted slot 0, and an active slot 1 */
    assert(pl_frame_compact(test_frame) == 0);
    
    /* After compaction, fragmented space should be wiped clean */
    assert(pl_frame_get_fragmented_space(test_frame) == 0);
    
    /* The slot count should compress down to 1 (since slot 0 was removed) */
    assert(pl_frame_get_slot_count(test_frame) == 1);
    
    /* The data from the old slot 1 should now be at the new slot 0 position */
    new_start = pl_frame_slot_get_start(test_frame, 0);
    assert(memcmp(test_frame + new_start, tuple_b, 8) == 0);
}

int main(void) {
    /* Ensure a clean state by zeroing the buffer */
    memset(test_frame, 0, DB_PAGE_SIZE);

    /* Run tests sequentially as they build on the state of test_frame */
    test_pl_frame_create_success();
    
    test_pl_frame_insert_success();
    test_pl_frame_insert_out_of_bounds_error();
    
    test_pl_frame_update_success();
    test_pl_frame_update_size_mismatch_error();
    
    test_pl_frame_delete_success();
    test_pl_frame_compact_success();

    printf("PL - All assertions passed!\n");
    return 0;
}
