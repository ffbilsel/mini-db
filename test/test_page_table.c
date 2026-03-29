#include "page_table.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

PageTable pt;

void test_pt_put_and_get(void) {
    assert(pt_put(&pt, 5, 100) == 0);
    assert(pt_get(&pt, 5) == 100);
}

void test_pt_get_miss(void) {
    assert(pt_get(&pt, 10) == -1); 
}

void test_pt_update_existing(void) {
    assert(pt_put(&pt, 5, 200) == 0);
    assert(pt_get(&pt, 5) == 200);
}

void test_pt_remove(void) {
    pt_remove(&pt, 5);
    assert(pt_get(&pt, 5) == -1);
}

int main(void) {
    pt_init(&pt);
    
    test_pt_put_and_get();
    test_pt_get_miss();
    test_pt_update_existing();
    test_pt_remove();
    
    pt_clear(&pt);
    
    printf("PT - All assertions passed!\n");
    return 0;
}