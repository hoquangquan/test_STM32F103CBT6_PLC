#include "modbus_rtu.h"
#include "usart.h"
#include <string.h>

#define MODBUS_TIMEOUT 500

static uint8_t rxBuffer[256];
static uint16_t rxIndex = 0;
static uint8_t rxData;
static uint8_t isReceiving = 0;
static uint32_t lastRxTime = 0;

volatile uint16_t plc_registers[10] = {0};
volatile uint8_t modbus_rx_ready = 0;

static HAL_StatusTypeDef RS485_Transmit(uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status;

    /* SP3485E /RE and DE are tied to PA8: high = transmit, low = receive. */
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
    status = HAL_UART_Transmit(&huart1, data, length, MODBUS_TIMEOUT);
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);

    return status;
}

static uint16_t Modbus_CRC16(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void Modbus_Init(void)
{
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
    // Start listening for incoming bytes
    HAL_UART_Receive_IT(&huart1, &rxData, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if (rxIndex < sizeof(rxBuffer)) {
            rxBuffer[rxIndex++] = rxData;
        }
        lastRxTime = HAL_GetTick();
        isReceiving = 1;
        
        // Re-enable interrupt
        HAL_UART_Receive_IT(&huart1, &rxData, 1);
    }
}

void Modbus_Process(void)
{
    // Simple Modbus RTU end-of-frame detection via timeout (e.g., 5ms of silence)
    if (isReceiving && ((HAL_GetTick() - lastRxTime) > 5)) {
        isReceiving = 0; // Frame ended
        
        if (rxIndex == 6) {
            /* GP.OUTPUT sends K6 bytes. QJ71C24N nonprocedural data is
             * packed low byte first in each PLC word. */
            for (uint16_t i = 0; i < 3; i++) {
                plc_registers[i] = (uint16_t)rxBuffer[i * 2]
                                 | ((uint16_t)rxBuffer[i * 2 + 1] << 8);
            }
            modbus_rx_ready = 1;
        }
        rxIndex = 0; // Reset for next frame
    }
}

HAL_StatusTypeDef RawSerial_SendInputs(const uint16_t *values, uint16_t numValues)
{
    /* G.INPUT uses D4673 = K20, therefore every packet is 20 bytes. */
    uint8_t frame[20] = {0};
    uint16_t wordsToSend = (numValues > 10U) ? 10U : numValues;

    for (uint16_t i = 0; i < wordsToSend; i++) {
        frame[i * 2] = (uint8_t)(values[i] & 0xFFU);
        frame[i * 2 + 1] = (uint8_t)(values[i] >> 8);
    }

    rxIndex = 0;
    isReceiving = 0;
    return RS485_Transmit(frame, sizeof(frame));
}

HAL_StatusTypeDef Modbus_ReadHoldingRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t numRegs)
{
    uint8_t frame[8];
    frame[0] = slaveAddr;
    frame[1] = 0x03; 
    frame[2] = startAddr >> 8;
    frame[3] = startAddr & 0xFF;
    frame[4] = numRegs >> 8;
    frame[5] = numRegs & 0xFF;
    
    uint16_t crc = Modbus_CRC16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;
    
    rxIndex = 0; // Reset buffer before sending request
    isReceiving = 0;
    modbus_rx_ready = 0;
    
    return RS485_Transmit(frame, 8);
}

HAL_StatusTypeDef Modbus_WriteSingleRegister(uint8_t slaveAddr, uint16_t regAddr, uint16_t regValue)
{
    uint8_t frame[8];
    frame[0] = slaveAddr;
    frame[1] = 0x06; 
    frame[2] = regAddr >> 8;
    frame[3] = regAddr & 0xFF;
    frame[4] = regValue >> 8;
    frame[5] = regValue & 0xFF;
    
    uint16_t crc = Modbus_CRC16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;
    
    rxIndex = 0; // Reset buffer before sending request
    isReceiving = 0;
    modbus_rx_ready = 0;
    
    return RS485_Transmit(frame, 8);
}

HAL_StatusTypeDef Modbus_WriteMultipleRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t numRegs, uint16_t* values)
{
    uint8_t frame[256];
    uint8_t byteCount = numRegs * 2;
    
    frame[0] = slaveAddr;
    frame[1] = 0x10; 
    frame[2] = startAddr >> 8;
    frame[3] = startAddr & 0xFF;
    frame[4] = numRegs >> 8;
    frame[5] = numRegs & 0xFF;
    frame[6] = byteCount;
    
    for (uint16_t i = 0; i < numRegs; i++) {
        frame[7 + i*2] = values[i] >> 8;
        frame[8 + i*2] = values[i] & 0xFF;
    }
    
    uint16_t frameLen = 7 + byteCount;
    uint16_t crc = Modbus_CRC16(frame, frameLen);
    frame[frameLen] = crc & 0xFF;
    frame[frameLen + 1] = crc >> 8;
    
    rxIndex = 0; // Reset buffer before sending request
    isReceiving = 0;
    modbus_rx_ready = 0;
    
    return RS485_Transmit(frame, frameLen + 2);
}
