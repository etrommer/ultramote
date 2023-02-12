#include "flash_backup.h"
#include <stdbool.h>

extern char _flash_backup_page;
const flash_backup_t *backup_page = (flash_backup_t *)&_flash_backup_page;
const size_t LAST_SLOT = FLASH_PAGE_SIZE / sizeof(flash_backup_t) - 1;

bool is_valid(flash_backup_t *slot)
{
    return slot->channel <= 0x0f;
}

// Find the slot in the page that contains the current configuration
size_t find_current_slot()
{
    for (size_t i = 0; i <= LAST_SLOT; i++)
    {
        if (!is_valid(backup_page + i))
        {
            return i - 1;
        }
    }
    return LAST_SLOT;
}

flash_backup_status_t flash_backup_write(flash_backup_t *backup_data)
{
    if (!is_valid(backup_data))
    {
        return FLASH_BACKUP_INVALID_DATA;
    }

    size_t current_slot = find_current_slot();
    if (current_slot == LAST_SLOT)
    {
        // TODO: No empty slot in page left. Erase page.
        current_slot = -1;
    }
    current_slot += 1;
    // TODO Write to Flash at 'current_slot'

    return FLASH_BACKUP_OK;
}

const flash_backup_t *flash_backup_read(void)
{
    size_t current_slot = find_current_slot();
    if (current_slot == -1)
    {
        // TODO: Page is uninitialized
    }
    return backup_page + current_slot;
}