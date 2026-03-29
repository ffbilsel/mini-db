#include "page_layout.h"

static uint64_t read_bytes(uint8_t page[], uint16_t offset, uint16_t byte_size) {
    uint64_t res = 0;
    uint16_t i; 
    for(i = 0; i < byte_size; i++) {
        res = (res << 8) | page[offset + i];
    }
    return res;
}

static void write_bytes(uint8_t page[], uint16_t offset, uint64_t data, uint8_t byte_size) {
    uint16_t i; 
    for(i = 0; i < byte_size; i++) {
        uint8_t shift_amount = 8 * (byte_size - 1 - i);
        page[offset + i] = (uint8_t)((data >> shift_amount) & 0xFF);
    }
}

static uint16_t pl_get_slot_count(uint8_t page[]) {
    return read_bytes(page, SLOT_COUNT_OFFSET, 2);
}

static uint16_t pl_get_free_space_pointer(uint8_t page[]) {
    return read_bytes(page, FREE_SPACE_POINTER_OFFSET, 2);
}

static uint16_t pl_get_fragmented_space(uint8_t page[]) {
    return read_bytes(page, FRAGMENTED_SPACE_OFFSET, 2);
}

static uint16_t pl_get_free_space(uint8_t page[]) {
    uint16_t slot_array_end = PL_HEADER_SIZE + (4 * pl_get_slot_count(page));
    return pl_get_free_space_pointer(page) - slot_array_end;
}

static uint16_t pl_slot_get_start(uint8_t page[], uint16_t slot_id) {
    uint16_t slot_offset = PL_HEADER_SIZE + 4 * slot_id;
    return read_bytes(page, slot_offset, 2);
}

static uint16_t pl_slot_get_size(uint8_t page[], uint16_t slot_id) {
    uint16_t slot_offset = PL_HEADER_SIZE + 4 * slot_id;
    return read_bytes(page, slot_offset + 2, 2);
}

static void pl_update_slot(uint8_t page[], uint16_t slot_id, uint16_t new_start, uint16_t new_size) {
    uint16_t slot_array_offset = PL_HEADER_SIZE + 4 * slot_id;
    write_bytes(page, slot_array_offset, new_start, 2);
    write_bytes(page, slot_array_offset + 2, new_size, 2);
}

int pl_insert_tuple(uint8_t page[], uint8_t *tuple, uint16_t size) {
    uint16_t required_space;
    uint16_t offset;
    uint16_t new_slot_count;
    uint16_t free_space;
    int status;

    required_space = size + 4; /* Data size + 4 bytes for the slot array entry */
    free_space = pl_get_free_space(page);
    if(required_space > free_space) {
        if(pl_get_fragmented_space(page) + free_space > required_space) {
            status = pl_defragment_page(page);
            if(status != 0) return 1;
            return pl_insert_tuple(page, tuple, size);
        }
        return 1; /* Not enough room */
    }

    offset = pl_get_free_space_pointer(page) - size;
    memcpy(page + offset, tuple, size);

    write_bytes(page, FREE_SPACE_POINTER_OFFSET, offset, 2);

    new_slot_count = pl_get_slot_count(page) + 1; 
    write_bytes(page, SLOT_COUNT_OFFSET, new_slot_count, 2);

    pl_update_slot(page, new_slot_count - 1, offset, size);

    return 0;
}

int pl_update_tuple(uint8_t page[], uint16_t slot_id, uint8_t *tuple, uint16_t new_size) {
    uint16_t slot_count;
    uint16_t slot_start;
    uint16_t slot_size;

    slot_count = pl_get_slot_count(page);
    if(slot_count <= slot_id) return 1;

    slot_start = pl_slot_get_start(page, slot_id);
    slot_size = pl_slot_get_size(page, slot_id);
    
    if(new_size != slot_size) return 1;

    memcpy(page + slot_start, tuple,slot_size);

    return 0;
}

int pl_delete_tuple(uint8_t page[], uint16_t slot_id) {
    uint16_t slot_start;
    uint16_t slot_size;

    slot_start = pl_slot_get_start(page, slot_id);
    if(slot_start == 0) return 1;
    
    slot_size = pl_slot_get_size(page, slot_id);
    pl_update_slot(page, slot_id, 0, 0);
    write_bytes(page, FRAGMENTED_SPACE_OFFSET, pl_get_fragmented_space(page) + slot_size, 2);
    
    return 1;
}

int pl_defragment_page(uint8_t page[]) {
    uint16_t free_space_pointer;
    uint16_t used_data_size;
    uint16_t slot_count;
    uint8_t *data_snapshot;
    uint16_t i;
    uint16_t slot_start;

    uint16_t slot_size;
    uint16_t target_offset;
    uint16_t new_slot_count;

    free_space_pointer = pl_get_free_space_pointer(page);
    
    /* The actual size of the tuples sitting at the end of the page */
    used_data_size = DB_PAGE_SIZE - free_space_pointer;
    slot_count = pl_get_slot_count(page);

    /* If there is no data to compact, just clear fragmentation and return */
    if(used_data_size == 0) {
        write_bytes(page, FRAGMENTED_SPACE_OFFSET, 0, 2);
        return 0;
    }

    data_snapshot = (uint8_t*)malloc(used_data_size);
    if(!data_snapshot) return 1;

    memcpy(data_snapshot, page + free_space_pointer, used_data_size);
    
    target_offset = DB_PAGE_SIZE;
    new_slot_count = 0;
    
    for(i = 0; i < slot_count; i++) {
        slot_start = pl_slot_get_start(page, i);
        slot_size = pl_slot_get_size(page, i);
        
        if(slot_start != 0) { 
            target_offset -= slot_size;
            
            memcpy(page + target_offset, data_snapshot + slot_start - free_space_pointer, slot_size);
            
            pl_update_slot(page, new_slot_count, target_offset, slot_size);            
            new_slot_count++;
        }
    }

    write_bytes(page, SLOT_COUNT_OFFSET, new_slot_count, 2);
    write_bytes(page, FREE_SPACE_POINTER_OFFSET, target_offset, 2);
    write_bytes(page, FRAGMENTED_SPACE_OFFSET, 0, 2);
    
    free(data_snapshot);
    
    return 0;
}

void pl_set_page_layout(uint8_t *page, uint32_t id) {
    write_bytes(page, PAGE_ID_OFFSET, id, 4);
    write_bytes(page, LSN_OFFSET, 0, 8);
    write_bytes(page, SLOT_COUNT_OFFSET, 0, 2);
    write_bytes(page, FREE_SPACE_POINTER_OFFSET, DB_PAGE_SIZE, 2); 
    write_bytes(page, FRAGMENTED_SPACE_OFFSET, 0, 2);
    write_bytes(page, FLAGS_OFFSET, 0, 4);
}
