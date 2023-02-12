#pragma once
#include <main.h>

typedef struct {
    uint8_t channel;
} flash_backup_t;

typedef enum {
    FLASH_BACKUP_OK,
    FLASH_BACKUP_INVALID_DATA,
    FLASH_BACKUP_WRITE_ERROR
} flash_backup_status_t;

flash_backup_status_t flash_backup_write(flash_backup_t *backup_data);
const flash_backup_t *flash_backup_read(void);