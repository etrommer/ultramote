#pragma once
#include <stdbool.h>

// "BOOT" in ASCII
#define BOOTLOADER_MAGIC 0x424F4F54

void bootloader_enter(void);
void bootloader_check(void);