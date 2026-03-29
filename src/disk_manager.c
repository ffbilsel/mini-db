#include "disk_manager.h"

uint32_t db_page_count = 0;

static FILE *dm_writer = NULL;
static FILE *dm_reader = NULL;

static int dm_write_page_wo_flush(Frame* frame) {
    int seek_result = -1;

    if(dm_writer == NULL || frame == NULL || (frame->state != FRAME_STATE_NEW && frame->page_id >= db_page_count)) return 1;

    if(frame->state == FRAME_STATE_NEW) seek_result = fseek(dm_writer, 0, SEEK_END);
    else seek_result = fseek(dm_writer, (long)frame->page_id * DB_PAGE_SIZE, SEEK_SET);
    if(seek_result != 0) return 1;

    if(fwrite(frame->page, DB_PAGE_SIZE, 1, dm_writer) != 1) return 1;
    
    if(frame->state == FRAME_STATE_NEW) {
        db_page_count++;
        frame->page_id = db_page_count;
    }
    return 0; 
}

Frame* dm_write_pages(Frame* frame) {
    Frame* temp;
    while(frame != NULL) {
        int status = dm_write_page_wo_flush(frame);
        if(status != 0) break;
        
        temp = frame;
        frame = frame->next;
        free(temp); 
    }
    fflush(dm_writer);
    return frame;
}

/* Reads raw page data directly into an existing buffer */
int dm_read_page_into(uint8_t *dest, uint32_t page_id) {
    if (dm_reader == NULL || page_id >= db_page_count) return 1;
    if (fseek(dm_reader, (long)page_id * DB_PAGE_SIZE, SEEK_SET) != 0) return 1;
    if (fread(dest, DB_PAGE_SIZE, 1, dm_reader) != 1) return 1;
    return 0;
}

/* Exposes single page write for the Buffer Pool Manager's eviction cycle */
int dm_write_page(Frame *frame) {
    int status = dm_write_page_wo_flush(frame);
    if (status == 0) fflush(dm_writer);
    return status;
}

int dm_init(void) {
    int seek_result;
    dm_writer = fopen(DM_FILENAME, "r+b");
    if(dm_writer == NULL) dm_writer = fopen(DM_FILENAME, "w+b");
    
    dm_reader = fopen(DM_FILENAME, "rb");
    if(dm_writer == NULL || dm_reader == NULL) return -1;     
    seek_result = fseek(dm_reader, 0, SEEK_END);
    if(seek_result != 0) return -1;
    db_page_count = ftell(dm_reader) / DB_PAGE_SIZE;
    return db_page_count; 
}

int dm_close(void) {
    if(dm_writer == NULL || dm_reader == NULL) return 1;
    fclose(dm_writer);
    fclose(dm_reader);
    return 0;
}

int dm_clear(void) {
    FILE* temp = fopen(DM_FILENAME, "wb");
    if(temp == NULL) return 1;
    fclose(temp);
    return 0;
}
