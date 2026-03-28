#include "disk_manager.h"

/* Define the global here */
uint32_t db_page_count = 0;

static FILE *dm_writer = NULL;
static FILE *dm_reader = NULL;

uint8_t* dm_page_read(uint32_t page_id) {
    uint8_t *buffer = (uint8_t *)malloc(DB_PAGE_SIZE * sizeof(uint8_t));
    if (buffer == NULL || dm_reader == NULL || page_id >= db_page_count) {
        free(buffer);
        return NULL;
    }
    if (fseek(dm_reader, (long)page_id * DB_PAGE_SIZE, SEEK_SET) != 0) {
        free(buffer);
        return NULL;
    }
    if (fread(buffer, DB_PAGE_SIZE, 1, dm_reader) != 1) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

PageNode* dm_page_write_bulk(PageNode* page_node) {
    PageNode* temp;
    while (page_node != NULL) {
        int status = dm_page_write(page_node->page, page_node->page_id, page_node->is_new);
        if (status != 0) break;
        
        temp = page_node;
        page_node = page_node->next;
        free(temp); /* Do not free temp->page since it's an array inside the struct */
    }
    fflush(dm_writer);
    return page_node;
}

int dm_page_write(uint8_t* buffer, uint32_t page_id, int is_new) {
    int seek_result = -1;
    if (buffer == NULL || dm_writer == NULL || (is_new == 0 && page_id >= db_page_count)) return 1;

    if (is_new == 1) seek_result = fseek(dm_writer, 0, SEEK_END);
    else seek_result = fseek(dm_writer, (long)page_id * DB_PAGE_SIZE, SEEK_SET);
    
    if (seek_result != 0) return 1;

    if (fwrite(buffer, DB_PAGE_SIZE, 1, dm_writer) != 1) return 1;
    
    if (is_new == 1) db_page_count++; 
    return 0; 
}

int dm_init(void) {
    /* Try to open existing file first, otherwise create it */
    dm_writer = fopen(DM_FILENAME, "r+b");
    if (dm_writer == NULL) {
        dm_writer = fopen(DM_FILENAME, "w+b");
    }
    
    dm_reader = fopen(DM_FILENAME, "rb");
    if (dm_writer == NULL || dm_reader == NULL) {
        return -1; 
    }
    
    fseek(dm_reader, 0, SEEK_END);
    db_page_count = ftell(dm_reader) / DB_PAGE_SIZE;
    return db_page_count; 
}

int dm_close(void) {
    if (dm_writer == NULL || dm_reader == NULL) return 1;
    fclose(dm_writer);
    fclose(dm_reader);
    return 0;
}

int dm_clear(void) {
    FILE* temp = fopen(DM_FILENAME, "wb");
    if (temp == NULL) return 1;
    fclose(temp);
    return 0;
}
