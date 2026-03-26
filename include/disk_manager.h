#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "db_global.h"

#define DM_FILENAME "main.db"

int dm_page_read(char *buffer, uint32_t page_id);
int dm_page_write(char *buffer, uint32_t page_id, int is_append);

int dm_init(void);
int dm_clear(void);
int dm_close(void);

int dm_page_count_get(void);

#endif /* DISK_MANAGER_H */
