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
        
        if (rxIndex >= 4) {
            uint16_t crc_calc = Modbus_CRC16(rxBuffer, rxIndex - 2);
            uint16_t crc_recv = rxBuffer[rxIndex - 2] | (rxBuffer[rxIndex - 1] << 8);
            
            if (crc_calc == crc_recv) {
                // Good frame, check function code
                if (rxBuffer[1] == 0x03) { // Read Holding Registers response
                    uint8_t byteCount = rxBuffer[2];
                    if (rxIndex >= (3 + byteCount + 2)) {
                        for (int i = 0; i < (byteCount / 2) && i < 10; i++) {
                            plc_registers[i] = (rxBuffer[3 + i * 2] << 8) | rxBuffer[4 + i * 2];
                        }
                        modbus_rx_ready = 1;
                    }
                }
                else if (rxBuffer[1] == 0x06) { // Write Single Register response
                    modbus_rx_ready = 1;
                }
            }
        }
        rxIndex = 0; // Reset for next frame
    }
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
    
    return HAL_UART_Transmit(&huart1, frame, 8, MODBUS_TIMEOUT);
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
    
    return HAL_UART_Transmit(&huart1, frame, 8, MODBUS_TIMEOUT);
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
    
    return HAL_UART_Transmit(&huart1, frame, frameLen + 2, MODBUS_TIMEOUT);
}
