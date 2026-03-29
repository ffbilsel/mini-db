#include "disk_manager.h"
#include <assert.h>
#include <string.h>

Frame test_frame_prev;
Frame test_frame_next;

void help_read_and_compare(uint32_t page_id, uint8_t *expected_str) {
    uint8_t read_buf[DB_PAGE_SIZE];
    assert(dm_read_page_into(read_buf, page_id) == 0);
    assert(memcmp(expected_str, read_buf, DB_PAGE_SIZE) == 0);
}

void init_success(void) {
    assert(dm_clear() == 0);
    assert(dm_init() == 0);

    memset(&test_frame_prev, 0, sizeof(Frame));
    memset(&test_frame_next, 0, sizeof(Frame));

    strcpy((char *)test_frame_prev.page, "TEST_PREV");
    test_frame_prev.page_id = 0;
    test_frame_prev.state = FRAME_STATE_NEW;

    strcpy((char *)test_frame_next.page, "TEST_NEXT");
    test_frame_next.page_id = 1;
    test_frame_next.state = FRAME_STATE_NEW;
}

void read_out_of_bounds_error(void) {
    uint8_t buf[DB_PAGE_SIZE];
    assert(dm_read_page_into(buf, 999) == 1);
}

void write_success(void) {
    assert(dm_write_page(&test_frame_prev) == 0);
    help_read_and_compare(0, test_frame_prev.page);
}

void append_success(void) {
    assert(dm_write_page(&test_frame_next) == 0);
    help_read_and_compare(1, test_frame_next.page);
}

void replace_success(void) {
    test_frame_next.page_id = 0;
    test_frame_next.state = FRAME_STATE_DIRTY; /* Overwrite existing */
    assert(dm_write_page(&test_frame_next) == 0);
    help_read_and_compare(0, test_frame_next.page);
}

int main(void) {
    init_success();
    read_out_of_bounds_error();
    write_success();
    append_success();
    replace_success();

    dm_close();
    printf("DM - All assertions passed!\n");
    return 0;
}