#include "buffer_pool_manager.h"
#include "disk_manager.h"
#include "page_layout.h" /* Need this for PL_HEADER_SIZE */
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_pm_init(void) {
    /* Need to initialize disk manager first so pm_fetch can actually read */
    assert(dm_clear() == 0);
    assert(dm_init() == 0);
    assert(pm_init() == 0);
}

void test_pm_create_and_fetch(void) {
    uint32_t page_id;
    uint8_t *fetched;

    uint8_t page_data[DB_PAGE_SIZE];
    memset(page_data, 0, DB_PAGE_SIZE);
    
    /* Write our test data safely AFTER the 22-byte page header */
    strcpy((char *)(page_data + PL_HEADER_SIZE), "BUFFER_POOL_TEST");

    /* Create a new page. This formats bytes 0-21 with the header. */
    page_id = pm_create_page(page_data);
    
    /* Fetch it back (should hit the cache) */
    fetched = pm_fetch_page(page_id);
    assert(fetched != NULL);
    
    /* Check for our string at the correct offset */
    assert(strncmp((char *)(fetched + PL_HEADER_SIZE), "BUFFER_POOL_TEST", 16) == 0);
    
    free(fetched);
}

void test_pm_close_flushes_dirty(void) {
    uint8_t *disk_read;
    /* pm_close should flush the dirty page created in test_pm_create_and_fetch to disk */
    assert(pm_close() == 0);
    
    /* Re-open disk manager to verify it was written */
    disk_read = dm_page_read(0); /* Assuming it was page 0 */
    assert(disk_read != NULL);
    
    /* Again, check after the header! */
    assert(strncmp((char *)(disk_read + PL_HEADER_SIZE), "BUFFER_POOL_TEST", 16) == 0);
    
    free(disk_read);
    dm_close();
}

int main(void) {
    test_pm_init();
    test_pm_create_and_fetch();
    test_pm_close_flushes_dirty();

    printf("PM - All assertions passed!\n");
    return 0;
}
