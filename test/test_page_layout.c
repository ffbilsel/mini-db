#include "page_layout.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

uint8_t test_page[DB_PAGE_SIZE];
uint8_t tuple_a[] = "TUPLE_A"; 
uint8_t tuple_b[] = "TUPLE_B"; 
uint8_t tuple_c[] = "TUPLE_C"; 
uint8_t tuple_mod[] = "MOD_TUP"; 

/* Note: Assuming you exposed getters like pl_page_get_slot_count in the header for testing, 
   or removed the 'static' keyword during compilation. */

void test_pl_page_create_success(void) {
    pl_page_create(test_page, 0);
}

void test_pl_page_insert_success(void) {
    assert(pl_page_insert_tuple(test_page, tuple_a, 8) == 0);
    assert(pl_page_insert_tuple(test_page, tuple_b, 8) == 0);
}

void test_pl_page_insert_out_of_bounds_error(void) {
    uint16_t massive_size = DB_PAGE_SIZE + 1;
    uint8_t massive_tuple[1]; 
    assert(pl_page_insert_tuple(test_page, massive_tuple, massive_size) == 1);
}

void test_pl_page_update_success(void) {
    assert(pl_page_update_tuple(test_page, 0, tuple_mod, 8) == 0);
}

void test_pl_page_update_size_mismatch_error(void) {
    assert(pl_page_update_tuple(test_page, 1, tuple_mod, 99) == 1);
}

void test_pl_page_delete_success(void) {
    /* Your code returns 1 on success for delete_tuple */
    assert(pl_page_delete_tuple(test_page, 0) == 1); 
}

void test_pl_page_compact_success(void) {
    assert(pl_page_compact(test_page) == 0);
}

int main(void) {
    memset(test_page, 0, DB_PAGE_SIZE);

    test_pl_page_create_success();
    test_pl_page_insert_success();
    test_pl_page_insert_out_of_bounds_error();
    test_pl_page_update_success();
    test_pl_page_update_size_mismatch_error();
    test_pl_page_delete_success();
    test_pl_page_compact_success();

    printf("PL - All assertions passed!\n");
    return 0;
}
