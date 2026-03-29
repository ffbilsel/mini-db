#include "page_layout.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

uint8_t test_page[DB_PAGE_SIZE];
uint8_t tuple_a[] = "TUPLE_A"; 
uint8_t tuple_b[] = "TUPLE_B"; 
uint8_t tuple_mod[] = "MOD_TUP"; 

void test_pl_set_page_layout(void) {
    pl_set_page_layout(test_page, 42);
}

void test_pl_insert_success(void) {
    assert(pl_insert_tuple(test_page, tuple_a, 8) == 0);
    assert(pl_insert_tuple(test_page, tuple_b, 8) == 0);
}

void test_pl_update_success(void) {
    assert(pl_update_tuple(test_page, 0, tuple_mod, 8) == 0);
}

void test_pl_delete_success(void) {
    assert(pl_delete_tuple(test_page, 0) == 1); 
}

void test_pl_defragment_success(void) {
    assert(pl_defragment_page(test_page) == 0);
}

int main(void) {
    memset(test_page, 0, DB_PAGE_SIZE);

    test_pl_set_page_layout();
    test_pl_insert_success();
    test_pl_update_success();
    test_pl_delete_success();
    test_pl_defragment_success();

    printf("PL - All assertions passed!\n");
    return 0;
}