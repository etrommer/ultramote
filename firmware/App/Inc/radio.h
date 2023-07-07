#pragma once
#include <stdint.h>
#include "radio_driver.h"

// Air time: 95.23 ms
#define RF_FREQUENCY 868000000     /* Hz */
#define TX_OUTPUT_POWER 17         /* dBm */
#define LORA_BANDWIDTH LORA_BW_125 /* Hz */
#define LORA_SPREADING_FACTOR LORA_SF9
#define LORA_CODINGRATE LORA_CR_4_5
#define LORA_PREAMBLE_LENGTH 8 /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT 5  /* Symbols */

#define RADIO_RXTX_SIZE 2

void radio_init(void);
void radio_receive(uint32_t timeout);
void radio_send(uint8_t *buffer, uint32_t len);
bool radio_has_data(void);
void radio_get_data(uint8_t *buffer, uint32_t *len);
