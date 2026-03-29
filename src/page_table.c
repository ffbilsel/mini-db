#include "page_table.h"
#include <stdlib.h>

static uint32_t get_idx(uint32_t key) {
    return key % HASH_TABLE_SIZE;
}

void pt_init(PageTable *pt) {
    int i;
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        pt->table[i] = NULL;
    }
}

/* Returns the frame_id, or -1 if not found */
int pt_get(PageTable *pt, uint32_t page_id) {
    uint32_t idx = get_idx(page_id);
    HashNode *curr = pt->table[idx];
    
    while (curr != NULL) {
        if (curr->page_id == page_id) {
            return curr->frame_id;
        }
        curr = curr->next;
    }
    return -1;
}

int pt_put(PageTable *pt, uint32_t page_id, int frame_id) {
    uint32_t idx = get_idx(page_id);
    HashNode *new_node;

    /* Check if it already exists to update */
    HashNode *curr = pt->table[idx];
    while (curr != NULL) {
        if (curr->page_id == page_id) {
            curr->frame_id = frame_id;
            return 0;
        }
        curr = curr->next;
    }

    /* Create new node */
    new_node = (HashNode *)malloc(sizeof(HashNode));
    if (new_node == NULL) return 1;

    new_node->page_id = page_id;
    new_node->frame_id = frame_id;
    new_node->next = pt->table[idx];
    pt->table[idx] = new_node;

    return 0;
}

void pt_remove(PageTable *pt, uint32_t page_id) {
    uint32_t idx = get_idx(page_id);
    HashNode *curr = pt->table[idx];
    HashNode *prev = NULL;

    while (curr != NULL) {
        if (curr->page_id == page_id) {
            if (prev == NULL) pt->table[idx] = curr->next;
            else prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void pt_clear(PageTable *pt) {
    int i;
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *curr = pt->table[i];
        while (curr != NULL) {
            HashNode *temp = curr;
            curr = curr->next;
            free(temp);
        }
        pt->table[i] = NULL;
    }
}
