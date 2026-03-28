#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include "db_global.h"

#define DM_FILENAME "main.db"

uint8_t* dm_page_read(uint32_t page_id);
PageNode* dm_page_write_bulk(PageNode* page_list);
int dm_page_write(uint8_t *buffer, uint32_t page_id, int is_new);

int dm_init(void);
int dm_close(void);
int dm_clear(void);

#endif /* DISK_MANAGER_H */
