# Encoder Initial Setup Parameters

Thong so da test thanh cong cho encoder CANopen trong project `test_IDENCODER`.

## Encoder Parameters

```text
VCC             : 8-30V
Protocol        : CANopen
CAN baudrate    : 250 kbps
Node ID         : 6
Position object : 0x6004:00
Position format : 32-bit little-endian
```

## STM32 CAN Parameters

Voi clock hien tai cua STM32F103:

```text
PCLK1             : 36 MHz
CAN Prescaler     : 8
Sync Jump Width   : 1 tq
Time Segment 1    : 13 tq
Time Segment 2    : 4 tq
CAN mode          : Normal
Auto Bus-Off      : Enable
Auto Retransmit   : Enable
Calculated baud   : 250 kbps
```

Cong thuc:

```text
Baudrate = 36 MHz / (8 * (1 + 13 + 4)) = 250 kbps
```

## CANopen IDs

Voi Node ID = 6:

```text
NMT command ID       : 0x000
SDO request ID       : 0x606
SDO response ID      : 0x586
TPDO1 default ID     : 0x186
Heartbeat/Boot-up ID : 0x706
```

## Commands Used

Start encoder operational:

```text
ID   : 0x000
Data : 01 06
```

Read position `0x6004:00`:

```text
ID   : 0x606
Data : 40 04 60 00 00 00 00 00
```

## Code Constants

Trong `Core/Src/main.c`:

```c
#define ENCODER_DEFAULT_NODE_ID       6U
#define CAN_DEFAULT_BAUD_INDEX        1U
```

Trong `Core/Src/can.c`:

```c
hcan.Init.Prescaler = 8;
hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
hcan.Init.AutoBusOff = ENABLE;
hcan.Init.AutoRetransmission = ENABLE;
```

