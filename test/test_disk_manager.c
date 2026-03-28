#include "disk_manager.h"
#include <assert.h>
#include <string.h>

uint8_t test_buffer_prev[DB_PAGE_SIZE];
uint8_t test_buffer_next[DB_PAGE_SIZE];

void help_read_and_compare(uint32_t page_id, uint8_t *expected_str) {
    uint8_t *read_buf = dm_page_read(page_id);
    assert(read_buf != NULL);
    assert(memcmp(expected_str, read_buf, DB_PAGE_SIZE) == 0);
    free(read_buf);
}

void init_success(void) {
    assert(dm_clear() == 0);
    assert(dm_init() == 0);

    memset(test_buffer_prev, 0, DB_PAGE_SIZE);
    memset(test_buffer_next, 0, DB_PAGE_SIZE);

    strcpy((char *)test_buffer_prev, "TEST_PREV");
    strcpy((char *)test_buffer_next, "TEST_NEXT");
}

void read_out_of_bounds_error(void) {
    uint8_t *read_buf = dm_page_read(999);
    assert(read_buf == NULL);
}

void write_success(void) {
    assert(dm_page_write(test_buffer_prev, 0, 1) == 0);
    help_read_and_compare(0, test_buffer_prev);
}

void append_success(void) {
    assert(dm_page_write(test_buffer_next, 0, 1) == 0);
    help_read_and_compare(1, test_buffer_next);
}

void replace_success(void) {
    assert(dm_page_write(test_buffer_next, 0, 0) == 0);
    help_read_and_compare(0, test_buffer_next);
}

void write_out_of_bounds_error(void) {
    uint8_t dummy[DB_PAGE_SIZE] = {0};
    assert(dm_page_write(dummy, 999, 0) == 1);
}

int main(void) {
    init_success();
    read_out_of_bounds_error();
    write_success();
    append_success();
    replace_success();
    write_out_of_bounds_error();

    dm_close();
    printf("DM - All assertions passed!\n");
    return 0;
}
