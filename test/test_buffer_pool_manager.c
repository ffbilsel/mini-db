#include "buffer_pool_manager.h"
#include "disk_manager.h"
#include "page_layout.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_pm_init(void) {
    assert(dm_clear() == 0);
    assert(dm_init() == 0);
    assert(pm_init() == 0);
}

void test_pm_create_and_fetch(void) {
    /* 1. Create a page */
    Frame *frame = pm_create_frame();
    assert(frame != NULL);
    uint32_t page_id = frame->page_id;
    
    /* Write data AFTER the 22-byte page layout header */
    strcpy((char *)(frame->page + PL_HEADER_SIZE), "BUFFER_POOL_TEST");
    
    /* IMPORTANT: Must unpin after creation/writing! Mark as dirty. */
    pm_unpin_frame(page_id, 1);
    
    /* 2. Fetch the page back (should be a cache hit) */
    Frame *fetched = pm_fetch_frame(page_id);
    assert(fetched != NULL);
    assert(strncmp((char *)(fetched->page + PL_HEADER_SIZE), "BUFFER_POOL_TEST", 16) == 0);
    
    /* Unpin after reading! */
    pm_unpin_frame(page_id, 0);
}

void test_pm_close_flushes_dirty(void) {
    assert(pm_close() == 0);
    
    /* Re-open disk manager to verify the dirty frame flushed */
    uint8_t disk_read[DB_PAGE_SIZE];
    assert(dm_read_page_into(disk_read, 0) == 0);
    assert(strncmp((char *)(disk_read + PL_HEADER_SIZE), "BUFFER_POOL_TEST", 16) == 0);
    
    dm_close();
}

int main(void) {
    test_pm_init();
    test_pm_create_and_fetch();
    test_pm_close_flushes_dirty();

    printf("PM - All assertions passed!\n");
    return 0;
}