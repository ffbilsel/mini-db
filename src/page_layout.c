#include "page_layout.h"

static uint64_t page_read_bytes(uint8_t page[], uint16_t offset, uint16_t byte_size) {
    uint64_t res = 0;
    uint16_t i; 
    for (i = 0; i < byte_size; i++) {
        res = (res << 8) | page[offset + i];
    }
    return res;
}

static void page_write_bytes_to_page(uint8_t page[], uint16_t offset, uint64_t data, uint8_t byte_size) {
    uint16_t i; 
    for (i = 0; i < byte_size; i++) {
        uint8_t shift_amount = 8 * (byte_size - 1 - i);
        page[offset + i] = (uint8_t)((data >> shift_amount) & 0xFF);
    }
}

static void util_buffer_write_section(uint8_t* dest, uint8_t* src, uint16_t dest_offset, uint16_t src_offset, uint16_t size) {
    memcpy(dest + dest_offset, src + src_offset, size);
}

static uint16_t pl_page_get_slot_count(uint8_t page[]) {
    return page_read_bytes(page, SLOT_COUNT_OFFSET, 2);
}

static uint16_t pl_page_get_free_space_pointer(uint8_t page[]) {
    return page_read_bytes(page, FREE_SPACE_POINTER_OFFSET, 2);
}

static uint16_t pl_page_get_fragmented_space(uint8_t page[]) {
    return page_read_bytes(page, FRAGMENTED_SPACE_OFFSET, 2);
}

static uint16_t pl_page_get_free_space(uint8_t page[]) {
    uint16_t slot_array_end = PL_HEADER_SIZE + (4 * pl_page_get_slot_count(page));
    return pl_page_get_free_space_pointer(page) - slot_array_end;
}

static uint16_t pl_page_slot_get_start(uint8_t page[], uint16_t slot_id) {
    uint16_t slot_offset = PL_HEADER_SIZE + 4 * slot_id;
    return page_read_bytes(page, slot_offset, 2);
}

static uint16_t pl_page_slot_get_size(uint8_t page[], uint16_t slot_id) {
    uint16_t slot_offset = PL_HEADER_SIZE + 4 * slot_id;
    return page_read_bytes(page, slot_offset + 2, 2);
}

static void pl_page_update_slot(uint8_t page[], uint16_t slot_id, uint16_t new_start, uint16_t new_size) {
    uint16_t slot_array_offset = PL_HEADER_SIZE + 4 * slot_id;
    page_write_bytes_to_page(page, slot_array_offset, new_start, 2);
    page_write_bytes_to_page(page, slot_array_offset + 2, new_size, 2);
}

int pl_page_insert_tuple(uint8_t page[], uint8_t *tuple, uint16_t size) {
    /* C89: All variables declared at the top */
    uint16_t required_space;
    uint16_t offset;
    uint16_t new_slot_count;
    uint16_t free_space;

    required_space = size + 4; /* Data size + 4 bytes for the slot array entry */
    free_space = pl_page_get_free_space(page);
    if (required_space > free_space) {
        if(pl_page_get_fragmented_space(page) + free_space > required_space) {
            int status = pl_page_compact(page);
            if (status != 0) {
                return 1;
            }
            return pl_page_insert_tuple(page, tuple, size);
        }
        return 1; /* Not enough room */
    }

    offset = pl_page_get_free_space_pointer(page) - size;
    util_buffer_write_section(page, tuple, offset, 0, size);

    page_write_bytes_to_page(page, FREE_SPACE_POINTER_OFFSET, offset, 2);

    new_slot_count = pl_page_get_slot_count(page) + 1; 
    page_write_bytes_to_page(page, SLOT_COUNT_OFFSET, new_slot_count, 2);

    pl_page_update_slot(page, new_slot_count - 1, offset, size);

    return 0;
}

int pl_page_update_tuple(uint8_t page[], uint16_t slot_id, uint8_t *tuple, uint16_t new_size) {
    /* C89: All variables declared at the top */
    uint16_t slot_count;
    uint16_t slot_start;
    uint16_t slot_size;

    slot_count = pl_page_get_slot_count(page);
    if (slot_count <= slot_id) {
        return 1;
    }

    slot_start = pl_page_slot_get_start(page, slot_id);
    slot_size = pl_page_slot_get_size(page, slot_id);
    
    if (new_size != slot_size) {
        return 1;
    }

    util_buffer_write_section(page, tuple, slot_start, 0, slot_size);

    return 0;
}

int pl_page_delete_tuple(uint8_t page[], uint16_t slot_id) {
    /* C89: All variables declared at the top */
    uint16_t slot_start;
    uint16_t slot_size;

    slot_start = pl_page_slot_get_start(page, slot_id);
    if (slot_start == 0) {
        return 1;
    }
    
    slot_size = pl_page_slot_get_size(page, slot_id);
    pl_page_update_slot(page, slot_id, 0, 0);
    page_write_bytes_to_page(page, FRAGMENTED_SPACE_OFFSET, pl_page_get_fragmented_space(page) + slot_size, 2);
    
    return 1;
}

int pl_page_compact(uint8_t page[]) {
    /* C89: Declarations at the top */
    uint16_t free_space_pointer;
    uint16_t used_data_size;
    uint16_t slot_count;
    uint8_t *data_snapshot;
    uint16_t i;
    uint16_t slot_start;
    uint16_t slot_size;
    uint16_t target_offset;
    uint16_t new_slot_count;

    free_space_pointer = pl_page_get_free_space_pointer(page);
    
    /* The actual size of the tuples sitting at the end of the page */
    used_data_size = DB_PAGE_SIZE - free_space_pointer;
    slot_count = pl_page_get_slot_count(page);

    /* If there is no data to compact, just clear fragmentation and return */
    if (used_data_size == 0) {
        page_write_bytes_to_page(page, FRAGMENTED_SPACE_OFFSET, 0, 2);
        return 0;
    }

    /* Allocate exactly enough memory to back up the active tuples */
    data_snapshot = (uint8_t *)malloc(used_data_size);
    if (!data_snapshot) return 1;

    /* BACKUP: Copy FROM page TO data_snapshot */
    /* dest=data_snapshot, src=page, dest_offset=0, src_offset=free_space_pointer */
    util_buffer_write_section(data_snapshot, page, 0, free_space_pointer, used_data_size);
    
    target_offset = DB_PAGE_SIZE;
    new_slot_count = 0;
    
    for (i = 0; i < slot_count; i++) {
        slot_start = pl_page_slot_get_start(page, i);
        slot_size = pl_page_slot_get_size(page, i);
        
        if (slot_start != 0) { /* If the slot hasn't been deleted */
            target_offset -= slot_size;
            
            /* RESTORE: Copy FROM data_snapshot TO page */
            /* The tuple's position in the snapshot is (slot_start - free_space_pointer) */
            util_buffer_write_section(page, data_snapshot, target_offset, slot_start - free_space_pointer, slot_size);
            
            pl_page_update_slot(page, new_slot_count, target_offset, slot_size);            
            new_slot_count++;
        }
    }

    /* Update the header to reflect the newly compacted state */
    page_write_bytes_to_page(page, SLOT_COUNT_OFFSET, new_slot_count, 2);
    page_write_bytes_to_page(page, FREE_SPACE_POINTER_OFFSET, target_offset, 2);
    page_write_bytes_to_page(page, FRAGMENTED_SPACE_OFFSET, 0, 2);
    
    free(data_snapshot);
    
    return 0;
}

// Change signature to accept the ID
void pl_page_create(uint8_t *new_page, uint32_t page_id) {
    page_write_bytes_to_page(new_page, PAGE_ID_OFFSET, page_id, 4);
    page_write_bytes_to_page(new_page, LSN_OFFSET, 0, 8);
    page_write_bytes_to_page(new_page, SLOT_COUNT_OFFSET, 0, 2);
    page_write_bytes_to_page(new_page, FREE_SPACE_POINTER_OFFSET, DB_PAGE_SIZE, 2); 
    page_write_bytes_to_page(new_page, FRAGMENTED_SPACE_OFFSET, 0, 2);
    page_write_bytes_to_page(new_page, FLAGS_OFFSET, 0, 4);
}
