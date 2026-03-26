#include "disk_manager.h"
#include <assert.h>

char data_buffer[DB_PAGE_SIZE];
char test_buffer_prev[DB_PAGE_SIZE];
char test_buffer_next[DB_PAGE_SIZE];

void help_read_and_compare(int page_id, char *str) {
    assert(dm_page_read(data_buffer, page_id) == 0);
    assert(strncmp(str, data_buffer, DB_PAGE_SIZE) == 0);
}

void init_success(void) {
    assert(dm_clear() == 0);
    assert(dm_init() == 0);

    memset(data_buffer, 0, DB_PAGE_SIZE);

    sprintf(test_buffer_prev, "%s", "TEST_PREV");
    sprintf(test_buffer_next, "%s", "TEST_NEXT");

}

void read_out_of_bounds_error(void) {
    assert(dm_page_read(data_buffer, 999) == 1);
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
    assert(dm_page_write(data_buffer, 999, 0) == 1);
}

int main(void) {
    
    init_success();
    read_out_of_bounds_error();
    write_success();
    append_success();
    replace_success();
    write_out_of_bounds_error();

    printf("DM - All assertions passed!\n");

}
