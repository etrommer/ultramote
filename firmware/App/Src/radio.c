#include <string.h>
#include "radio.h"

static PacketParams_t packetParams;
const RadioLoRaBandwidths_t Bandwidths[] = {LORA_BW_125, LORA_BW_250, LORA_BW_500};
uint8_t rx_buffer[RADIO_RXTX_SIZE];
uint8_t rx_buffer_len = 0;

static void handle_received(void)
{
    PacketStatus_t packetStatus;

    // Workaround 15.3 in DS.SX1261-2.W.APP (because following RX w/ timeout sequence)
    SUBGRF_WriteRegister(0x0920, 0x00);
    SUBGRF_WriteRegister(0x0944, (SUBGRF_ReadRegister(0x0944) | 0x02));

    SUBGRF_GetPayload(rx_buffer, &rx_buffer_len, RADIO_RXTX_SIZE);
    SUBGRF_GetPacketStatus(&packetStatus);

    // sprintf(uartBuff, "RssiValue=%d dBm, SnrValue=%d Hz\r\n", packetStatus.Params.LoRa.RssiPkt, packetStatus.Params.LoRa.SnrPkt);
}

/**
 * @brief  Receive data trough SUBGHZSPI peripheral
 * @param  radioIrq  interrupt pending status information
 * @retval None
 */
static void RadioOnDioIrq(RadioIrqMasks_t radioIrq)
{
    switch (radioIrq)
    {
    case IRQ_TX_DONE:
        radio_receive(0);
        break;
    case IRQ_RX_DONE:
        handle_received();
        break;
    case IRQ_RX_TX_TIMEOUT:
        break;
    case IRQ_CRC_ERROR:
        break;
    default:
        break;
    }
}

void radio_init(void)
{
    // Initialize the hardware (SPI bus, TCXO control, RF switch)
    SUBGRF_Init(RadioOnDioIrq);

    // Use DCDC converter if `DCDC_ENABLE` is defined in radio_conf.h
    // "By default, the SMPS clock detection is disabled and must be enabled before enabling the SMPS." (6.1 in RM0453)
    SUBGRF_WriteRegister(SUBGHZ_SMPSC0R, (SUBGRF_ReadRegister(SUBGHZ_SMPSC0R) | SMPS_CLK_DET_ENABLE));
    SUBGRF_SetRegulatorMode();

    // Use the whole 256-byte buffer for both TX and RX
    SUBGRF_SetBufferBaseAddress(0x00, 0x00);

    SUBGRF_SetRfFrequency(RF_FREQUENCY);
    SUBGRF_SetRfTxPower(TX_OUTPUT_POWER);
    SUBGRF_SetStopRxTimerOnPreambleDetect(false);

    SUBGRF_SetPacketType(PACKET_TYPE_LORA);

    SUBGRF_WriteRegister(REG_LR_SYNCWORD, (LORA_MAC_PRIVATE_SYNCWORD >> 8) & 0xFF);
    SUBGRF_WriteRegister(REG_LR_SYNCWORD + 1, LORA_MAC_PRIVATE_SYNCWORD & 0xFF);

    ModulationParams_t modulationParams;
    modulationParams.PacketType = PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.Bandwidth = Bandwidths[LORA_BANDWIDTH];
    modulationParams.Params.LoRa.CodingRate = (RadioLoRaCodingRates_t)LORA_CODINGRATE;
    modulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
    modulationParams.Params.LoRa.SpreadingFactor = (RadioLoRaSpreadingFactors_t)LORA_SPREADING_FACTOR;
    SUBGRF_SetModulationParams(&modulationParams);

    packetParams.PacketType = PACKET_TYPE_LORA;
    packetParams.Params.LoRa.CrcMode = LORA_CRC_ON;
    packetParams.Params.LoRa.HeaderType = LORA_PACKET_FIXED_LENGTH;
    packetParams.Params.LoRa.InvertIQ = LORA_IQ_NORMAL;
    packetParams.Params.LoRa.PayloadLength = RADIO_RXTX_SIZE;
    packetParams.Params.LoRa.PreambleLength = LORA_PREAMBLE_LENGTH;
    SUBGRF_SetPacketParams(&packetParams);

    // SUBGRF_SetLoRaSymbNumTimeout(LORA_SYMBOL_TIMEOUT);

    // WORKAROUND - Optimizing the Inverted IQ Operation, see DS_SX1261-2_V1.2 datasheet chapter 15.4
    // RegIqPolaritySetup @address 0x0736
    SUBGRF_WriteRegister(0x0736, SUBGRF_ReadRegister(0x0736) | (1 << 2));
}

void radio_send(uint8_t *buffer, uint32_t len)
{
    SUBGRF_SetDioIrqParams(IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE);
    SUBGRF_SetSwitch(RFO_HP, RFSWITCH_TX);
    // Workaround 5.1 in DS.SX1261-2.W.APP (before each packet transmission)
    SUBGRF_WriteRegister(0x0889, (SUBGRF_ReadRegister(0x0889) | 0x04));
    packetParams.Params.LoRa.PayloadLength = len;
    SUBGRF_SetPacketParams(&packetParams);
    SUBGRF_SendPayload(buffer, len, 0);
}

void radio_receive(uint32_t timeout)
{
    SUBGRF_SetDioIrqParams(IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                           IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE);
    SUBGRF_SetSwitch(RFO_HP, RFSWITCH_RX);
    packetParams.Params.LoRa.PayloadLength = RADIO_RXTX_SIZE;
    SUBGRF_SetPacketParams(&packetParams);
    SUBGRF_SetRx(timeout << 6);
}

bool radio_has_data(void)
{
    return rx_buffer_len != 0;
}

void radio_get_data(uint8_t *buffer, uint32_t *len)
{
    memcpy(buffer, rx_buffer, rx_buffer_len);
    *len = rx_buffer_len;
    rx_buffer_len = 0;
}
