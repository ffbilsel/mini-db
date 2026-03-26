#include "disk_manager.h"

uint32_t dm_page_count = 0;

FILE *dm_writer = NULL;
FILE *dm_reader = NULL;

int dm_page_read(char *buffer, uint32_t page_id) {
    if (dm_reader == NULL || page_id >= dm_page_count) return 1;
    if (fseek(dm_reader, (long)page_id * DB_PAGE_SIZE, SEEK_SET) != 0) return 1;
    if (fread(buffer, DB_PAGE_SIZE, 1, dm_reader) != 1) return 1;
    return 0;
}

int dm_page_write(char *buffer, uint32_t page_id, int is_append) {
    int seek_result = -1;

    if (dm_writer == NULL || (is_append == 0 && page_id >= dm_page_count)) return 1;

    if (is_append == 1) {
        seek_result = fseek(dm_writer, 0, SEEK_END);
    } else {
        seek_result = fseek(dm_writer, (long)page_id * DB_PAGE_SIZE, SEEK_SET);
    }
    if (seek_result != 0) return 1;

    if (fwrite(buffer, DB_PAGE_SIZE, 1, dm_writer) != 1) return 1;
    
    fflush(dm_writer);
    
    dm_page_count += is_append == 1 ? 1 : 0; 
    return 0; 
}

int dm_init(void) {
    dm_writer = fopen(DM_FILENAME, "w+b");
    dm_reader = fopen(DM_FILENAME, "rb");
    
    if (dm_writer != NULL && dm_reader != NULL) {
        fseek(dm_reader, 0, SEEK_END);
        dm_page_count = ftell(dm_reader) / DB_PAGE_SIZE;
        return 0; 
    }
    return 1; 
}

int dm_clear(void) {
    return fopen(DM_FILENAME, "w") == NULL ? 1 : 0;
}

int dm_page_count_get(void) {
    return dm_page_count;
}

int dm_close(void) {
    if (dm_writer != NULL) {
        fclose(dm_writer);
    }
    if (dm_reader != NULL) {
        fclose(dm_reader);
    }
    return 0;
}
