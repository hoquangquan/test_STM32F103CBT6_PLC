#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void Modbus_Init(void);
void Modbus_Process(void);
HAL_StatusTypeDef RawSerial_SendInputs(const uint16_t *values, uint16_t numValues);

// Basic Modbus Master functions
HAL_StatusTypeDef Modbus_ReadHoldingRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t numRegs);
HAL_StatusTypeDef Modbus_WriteSingleRegister(uint8_t slaveAddr, uint16_t regAddr, uint16_t regValue);
HAL_StatusTypeDef Modbus_WriteMultipleRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t numRegs, uint16_t* values);

// Add custom logic in Modbus_Process for processing responses
extern volatile uint16_t plc_registers[10];
extern volatile uint8_t modbus_rx_ready;

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_RTU_H__ */
