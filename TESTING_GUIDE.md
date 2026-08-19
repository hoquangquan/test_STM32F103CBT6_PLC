# Hướng Dẫn Kịch Bản Test I/O Ảo Giữa STM32 và PLC Qua Modbus RTU

Tài liệu này mô tả chi tiết các bước thiết lập, đấu nối phần cứng và cách giám sát (sniffing) dữ liệu bằng USB-RS485 khi sử dụng mạch STM32 để mô phỏng và test 18 chân I/O của PLC Mitsubishi thông qua chuẩn truyền thông Modbus RTU.

---

## 1. Phương Thức Hoạt Động

Thay vì đấu nối dây điện vật lý trực tiếp từ các chân vi điều khiển vào các cổng X/Y của PLC, chúng ta sử dụng **phương pháp Test I/O Ảo qua Modbus RTU**.

*   **STM32 (Đóng vai trò Modbus Master):** Đọc trạng thái 8 nút nhấn/cảm biến vật lý trên bo mạch (I1-I8). Nó đóng gói dữ liệu này và gửi lệnh ghi (Mã lệnh `0x10`) xuống thanh ghi **D0** của PLC.
*   **PLC (Đóng vai trò Modbus Slave):** Liên tục nhận trạng thái đầu vào ảo từ thanh ghi **D0** để chạy chương trình Ladder. Sau khi tính toán logic, PLC xuất kết quả đầu ra ảo vào thanh ghi **D10**.
*   **Vòng lặp:** Mỗi 500ms, sau khi gửi dữ liệu đầu vào, STM32 sẽ gửi lệnh đọc (Mã lệnh `0x03`) thanh ghi **D10** của PLC để lấy kết quả. Dựa vào kết quả này, STM32 bật/tắt 8 đèn LED/Relay (O1-O8) tương ứng trên bo mạch.
*   **Máy Tính (Đóng vai trò Giám sát - Sniffer):** Sử dụng thiết bị chuyển đổi USB-RS485 gắn song song vào đường truyền để "nghe lén" dữ liệu truyền qua lại giữa STM32 và PLC mà không làm gián đoạn hệ thống.

---

## 2. Hướng Dẫn Đấu Nối Phần Cứng

Vì hệ thống dùng chuẩn **RS485 Half-Duplex (2 dây)**, toàn bộ các thiết bị (STM32, PLC, và USB-RS485) phải được đấu nối song song với nhau trên cùng một đường truyền (Bus).

### 2.1. Thiết lập trên PLC (Module QJ71C24N-R4)
Module của Mitsubishi hỗ trợ chuẩn 4 dây. Bắt buộc phải đấu nối (chập dây) ngay trên Domino của PLC để chuyển thành chuẩn 2 dây:
- Dùng một đoạn dây ngắn nối ngắt mạch chân **SDA** với **RDA** lại với nhau $\rightarrow$ Tạo thành chân **A (D+)**.
- Dùng một đoạn dây ngắn khác nối ngắt mạch chân **SDB** với **RDB** lại với nhau $\rightarrow$ Tạo thành chân **B (D-)**.

### 2.2. Đấu nối Mạng RS485 (Bus Topology)
Kéo dây kết nối song song cả 3 thiết bị theo sơ đồ sau:

| Tín Hiệu (Mạng) | Domino STM32 (Board) | Domino PLC QJ71C24N-R4 | Cục USB-RS485 trên PC |
| :--- | :--- | :--- | :--- |
| **Đường A (D+)** | Cắm vào chân **A** | Cắm vào cụm **SDA / RDA** | Cắm vào chân **A / D+** |
| **Đường B (D-)** | Cắm vào chân **B** | Cắm vào cụm **SDB / RDB** | Cắm vào chân **B / D-** |
| **Đường GND** | Cắm vào chân **GND** | Cắm vào chân **SG** | Cắm vào chân **GND** |

> [!CAUTION]
> Tuyệt đối không kết nối chéo A sang B. Sự nhầm lẫn này sẽ khiến hệ thống không thể truyền thông. Việc nối chung đường Mass (GND/SG) là cực kỳ quan trọng trong môi trường công nghiệp để tránh nhiễu và chênh lệch điện áp phá hỏng IC truyền thông.

---

## 3. Thiết Lập Hệ Thống

### 3.1. Thiết lập PLC (Qua phần mềm GX Works)
1.  Vào phần Parameter của module QJ71C24N-R4.
2.  Cấu hình cổng giao tiếp sang chế độ **Pre-defined Protocol** (hoặc Modbus RTU Slave).
3.  Cấu hình tốc độ truyền (Baudrate) khớp hoàn toàn với STM32:
    *   **Baudrate:** 9600 bps
    *   **Data bits:** 8
    *   **Parity:** None
    *   **Stop bits:** 1
4.  Trong chương trình Ladder, thực hiện ánh xạ (mapping) bit:
    *   Sử dụng lệnh để giải mã thanh ghi `D0` thành các bit (Ví dụ lấy `D0.0` sang `M0`) để dùng thay cho ngõ vào thực `X`.
    *   Sử dụng lệnh gom các bit logic ngõ ra (Ví dụ cuộn dây `M10`) và nạp vào thanh ghi `D10` (Ví dụ `D10.0`).

### 3.2. Thiết lập Phần mềm Giám sát trên Máy tính (Sniffer)
Sử dụng phần mềm `serial_modbus_tester` (hoặc các phần mềm Terminal Monitor như RealTerm, Hercules).

> [!WARNING]
> Phần mềm máy tính **BẮT BUỘC** phải được cấu hình ở chế độ chỉ đọc (Read-only / Listen / Monitor). Nếu phần mềm hoạt động như một Modbus Master (chủ động gửi lệnh ping/poll), nó sẽ gây ra xung đột tín hiệu điện áp (Collision) trên đường dây RS485 với con STM32.

**Cấu hình trên App:**
- Chọn cổng COM tương ứng của USB-RS485.
- Thông số: `9600` Baud, `8` Data, `None` Parity, `1` Stop bit.
- Định dạng hiển thị: **HEX** (Hexadecimal).

---

## 4. Kịch Bản Test Và Phân Tích Dữ Liệu

Khi cấp điện và chạy toàn bộ hệ thống, bạn sẽ thấy 2 cụm lệnh luân phiên xuất hiện trên màn hình máy tính (app monitor) cứ mỗi 500ms:

### Giai đoạn 1: STM32 ép Input ảo xuống PLC
- **(TX từ STM32):** `01 10 00 00 00 02 04 xx xx xx xx [CRC_L] [CRC_H]`
  - Ý nghĩa: STM32 đóng gói trạng thái 8 nút nhấn vật lý vào chùm byte `xx xx xx xx` và gửi mã hàm `0x10` yêu cầu ghi 2 thanh ghi (D0, D1).
- **(RX từ PLC):** `01 10 00 00 00 02 [CRC_L] [CRC_H]`
  - Ý nghĩa: PLC phản hồi xác nhận đã lưu thành công vào D0. Nếu không thấy chuỗi này tức là PLC chưa nhận được tín hiệu.

### Giai đoạn 2: STM32 đọc Output ảo từ PLC
- **(TX từ STM32):** `01 03 00 0A 00 02 [CRC_L] [CRC_H]`
  - Ý nghĩa: Gửi mã hàm `0x03` yêu cầu đọc 2 thanh ghi ngõ ra (D10, D11).
- **(RX từ PLC):** `01 03 04 yy yy yy yy [CRC_L] [CRC_H]`
  - Ý nghĩa: PLC gửi lại trạng thái tính toán của nó nằm trong chùm byte `yy yy yy yy`. STM32 sẽ đọc chùm byte này và bật/tắt các đèn LED tương ứng trên bo mạch.

**Đánh giá Test:** Nếu luồng dữ liệu trên phần mềm Sniffer chạy đều đặn và các LED trên bo STM32 phản hồi đúng logic mà bạn đã viết trong Ladder của PLC, điều đó chứng tỏ quá trình Test I/O qua Modbus đã hoàn toàn thành công!

---

## 5. Các Thông Số Có Thể Thay Đổi Để Đồng Bộ Với PLC
Trong quá trình test hoặc khi thay đổi thiết kế bên phía PLC, bạn có thể cần điều chỉnh lại các thông số sau trong dự án STM32:

### 5.1. Thông số Baudrate và Khung truyền (UART)
Nếu PLC bắt buộc phải chạy ở tốc độ khác (VD: 19200 bps) hoặc Parity khác (VD: Even):
- **Vị trí sửa:** Bật phần mềm **STM32CubeMX** (Mở file `test_IDENCODER.ioc`).
- **Thao tác:** Vào mục **Connectivity** $\rightarrow$ **USART1** $\rightarrow$ Chỉnh lại tham số trong bảng **Parameter Settings** (Baud Rate, Word Length, Parity).
- **Lưu ý:** Chỉnh xong phải bấm *Generate Code* và Build lại.

### 5.2. ID của trạm PLC (Slave Address)
Mặc định tôi đang để STM32 gọi trạm PLC số 1. Nếu bạn cài đặt QJ71C24N-R4 là trạm số khác (VD: Station 5):
- **Vị trí sửa:** File `Core/Src/main.c`, dòng code `uint8_t current_slave_address = 1;` (Khoảng dòng 38).
- **Thao tác:** Đổi số `1` thành số ID tương ứng của PLC.

### 5.3. Địa chỉ Thanh Ghi Input (Mặc định: D0)
Nếu bạn không muốn PLC nhận Input ảo ở D0 mà muốn chuyển sang thanh ghi khác (VD: D100):
- **Vị trí sửa:** File `Core/Src/main.c`, trong khối lệnh `if (test_state == 0)`.
- **Thao tác:** Tìm hàm `Modbus_WriteMultipleRegisters(current_slave_address, 0x0000, 2, virtual_inputs);`
- **Sửa lại:** Đổi `0x0000` thành mã HEX của địa chỉ mới (Ví dụ D100 hệ thập phân đổi sang HEX là `0x0064`).

### 5.4. Địa chỉ Thanh Ghi Output (Mặc định: D10)
Nếu bạn không muốn PLC trả kết quả ở D10 mà chuyển sang thanh ghi khác (VD: D200):
- **Vị trí sửa:** File `Core/Src/main.c`, trong khối lệnh `else if (test_state == 1)`.
- **Thao tác:** Tìm hàm `Modbus_ReadHoldingRegisters(current_slave_address, 0x000A, 2);`
- **Sửa lại:** Đổi `0x000A` thành mã HEX của địa chỉ mới (Ví dụ D200 hệ thập phân là `0x00C8`).

### 5.5. Chu kỳ quét dữ liệu (Polling Rate)
Mặc định STM32 đang quét (gửi lệnh) mỗi 500ms.
- **Vị trí sửa:** File `Core/Src/main.c`, dòng `if (HAL_GetTick() - last_modbus_poll >= 500)`
- **Thao tác:** Thay số `500` thành chu kỳ mong muốn (tính bằng mili-giây). Ví dụ muốn quét nhanh hơn thì để `100`, quét chậm thì để `1000`. (Lưu ý không nên để quá nhỏ vì có thể làm nghẽn đường truyền RS485).

---

# Cập Nhật Công Việc Và Kết Quả Sửa I/O PLC

## Công việc đã thực hiện

- Chuyển phương thức giao tiếp thực tế với QJ71C24N-R4 sang **RS485 Nonprocedural protocol** trên CH1, cấu hình `9600-8-N-1`, không parity và không sum check.
- Đấu RS485 hai dây: `SDA-RDA → A`, `SDB-RDB → B`, đồng thời nối chung `SG/GND`.
- Cấu hình PLC nhận gói input cố định 20 byte từ STM32 bằng `G.INPUT`:
  - `D4670 = 1` chọn CH1.
  - `D4673 = 20` quy định vùng nhận.
  - Dữ liệu input được lưu tại `D4680-D4690`.
- Cấu hình PLC gửi 6 byte output bằng `GP.OUTPUT`:
  - `D4660 = 1` chọn CH1.
  - `D4661 = 0` khi truyền bình thường.
  - `D4662 = 6` ở chế độ Byte unit.
  - Dữ liệu output lấy từ `D4650-D4652`.
- Đối chiếu sơ đồ `Board_Extension_AMR_v1.pdf` và xác định IC RS485 là `SP3485E`; hai chân `/RE` và `DE` nối chung với `PA8` của STM32.
- Cập nhật firmware để `PA8 = 1` trước khi truyền và trả về `PA8 = 0` ngay sau khi UART truyền xong, giúp STM32 nhả bus và nhận phản hồi từ PLC.

## Kết quả xác nhận

- PLC đã nhận đúng và ổn định trạng thái I1-I8 tại `D4680-D4690`.
- Ví dụ dữ liệu input active-low:
  - Không kích: `D4680 = 00FFH`.
  - Kích I1: `D4680 = 00FEH`.
  - Kích I2: `D4680 = 00FDH`.
  - Kích I8: `D4680 = 007FH`.
- PLC gửi đúng dữ liệu output từ `D4650-D4652`.
- Khi `D4650 = 0001H`, tool giám sát nhận đúng `01 00 00 00 00 00`.
- Dữ liệu trên PLC và tool giám sát đều đúng với kỳ vọng; giao tiếp hai chiều đã hoạt động ổn định.

## Công việc source code ngày 10/08/2026

### Tổng hợp chỉnh sửa cụ thể và mục đích

#### Chỉnh sửa trong source code STM32

| File/vị trí | Nội dung chỉnh sửa | Mục đích |
|---|---|---|
| `Core/Src/main.c` | Thêm `#include "modbus_rtu.h"` | Khắc phục lỗi compiler không nhận biết các hàm `Modbus_Init()`, `Modbus_Process()` và các biến `modbus_rx_ready`, `plc_registers`. |
| `Core/Src/main.c` | Đọc I1-I8 và ghép từng trạng thái vào các bit của `virtual_inputs[0]` | Chuyển trạng thái tám input vật lý thành một word để gửi sang PLC. |
| `Core/Src/main.c` | Thay lệnh ghi Modbus bằng `RawSerial_SendInputs(virtual_inputs, 2)` | Chuyển từ Modbus RTU sang dữ liệu raw, phù hợp với chế độ Nonprocedural và lệnh `G.INPUT` của QJ71C24N-R4. |
| `Core/Src/main.c` | Gọi gửi dữ liệu theo chu kỳ 600 ms | Tạo chu kỳ cập nhật input ổn định và dành thời gian để PLC gửi phản hồi trên bus half-duplex. |
| `Core/Src/main.c` | Khi `modbus_rx_ready = 1`, lấy `plc_registers[0]` và điều khiển O1-O8 | Chuyển word output PLC trả về thành tám output vật lý trên board STM32. |
| `Core/Inc/modbus_rtu.h` | Khai báo `RawSerial_SendInputs()` | Cho phép `main.c` gọi hàm gửi raw mà không phát sinh lỗi thiếu prototype. |
| `Core/Src/modbus_rtu.c` | Thêm `RawSerial_SendInputs()` và tạo frame cố định 20 byte | Đảm bảo mỗi chu kỳ STM32 gửi đúng kích thước mà PLC được cấu hình nhận. |
| `Core/Src/modbus_rtu.c` | Đóng gói word theo thứ tự low byte trước | Làm cho PLC nhận trực tiếp các giá trị `00FFH`, `00FEH`, `00FDH`... tại `D4680`. |
| `Core/Src/modbus_rtu.c` | Sửa `Modbus_Process()` để chỉ xử lý gói phản hồi đúng 6 byte | Phù hợp với `GP.OUTPUT`, `D4662 = 6` và loại bỏ các gói không đúng chiều dài. |
| `Core/Src/modbus_rtu.c` | Ghép mỗi hai byte nhận được thành một word `plc_registers[]` | Khôi phục chính xác dữ liệu các thanh ghi `D4650-D4652` do PLC gửi. |
| `Core/Inc/main.h` | Khai báo `RS485_DE_Pin` là `PA8` | Định danh chân điều khiển `/RE` và `DE` của IC SP3485E theo sơ đồ board. |
| `Core/Src/gpio.c` | Cấu hình PA8 là GPIO output, mặc định mức thấp | Đưa SP3485E về chế độ nhận khi khởi động, tránh chiếm giữ bus RS485. |
| `Core/Src/modbus_rtu.c` | Thêm hàm `RS485_Transmit()`; kéo PA8 lên trước khi gửi và xuống sau khi gửi xong | Điều khiển đúng hướng truyền half-duplex, giúp PLC có thể trả dữ liệu về mà không bị xung đột hoặc sai byte. |

#### Chỉnh sửa trong chương trình và cấu hình PLC

| Vị trí/thành phần | Nội dung chỉnh sửa | Mục đích |
|---|---|---|
| Switch Setting CH1 | Chọn `Independent`, `9600 bps`, `8 data bits`, `None parity`, `1 stop bit`, `None sum check` | Đồng bộ hoàn toàn định dạng UART giữa PLC, STM32 và USB-RS485. |
| Protocol CH1 | Chọn `Nonprocedural protocol` | Cho phép PLC nhận/gửi trực tiếp frame raw thay vì chờ hoặc phân tích frame Modbus. |
| Word/byte unit | Chọn `Byte` | Làm cho `D4662 = 6` tương ứng chính xác với sáu byte output. |
| Đấu dây QJ71C24N-R4 | Chập `SDA-RDA` thành A, chập `SDB-RDB` thành B và nối chung SG/GND | Chuyển cổng RS422/485 bốn dây thành bus RS485 hai dây half-duplex tương thích với board. |
| Buffer `U0\G164` | Ghi `K20` | Đặt điều kiện nhận đủ 20 byte cho mỗi gói input STM32, tránh dữ liệu của nhiều chu kỳ bị nối hoặc dịch vị trí. |
| Buffer `U0\G165` | Ghi `HFFFF` | Bỏ điều kiện kết thúc bằng CR/LF vì frame raw STM32 không chứa CR/LF. |
| Control nhận `D4670-D4673` | Đặt CH1 và dung lượng nhận 20 byte | Cung cấp đúng control data cho lệnh `G.INPUT`. |
| `G.INPUT U0 D4670 D4680 M820` | Nhận dữ liệu STM32 vào `D4680-D4690` | Đưa trạng thái I1-I8 vào vùng device PLC để chương trình Ladder sử dụng. |
| Control gửi `D4660-D4662` | Đặt CH1, xóa kết quả cũ và đặt chiều dài gửi 6 byte | Cung cấp đúng control data cho lệnh `GP.OUTPUT`. |
| Vùng `D4650-D4652` | Lưu word trạng thái output cần trả về | Tạo dữ liệu phản hồi để STM32 điều khiển O1-O8. |
| `GP.OUTPUT U0 D4660 D4650 M810` | Gửi sáu byte output sau khi PLC nhận xong input | Tổ chức truyền hai chiều theo thứ tự, hạn chế hai thiết bị phát đồng thời trên bus RS485. |

#### Chuỗi lệnh Ladder điều phối đã bổ sung/trao đổi

Phần nhận được khởi tạo và thực hiện theo cấu trúc:

```text
X3 ──[MOVP K1 D4670]

X4 ──[FMOVP K0 D4671 K2]
   ├─[MOVP K20 D4673]
   └─[G.INPUT U0 D4670 D4680 M820]
```

Mục đích:

- `MOVP K1 D4670`: chọn CH1 một lần tại cạnh lên của tín hiệu khởi tạo.
- `FMOVP K0 D4671 K2`: xóa kết quả và số lượng nhận cũ tại `D4671-D4672`.
- `MOVP K20 D4673`: đặt dung lượng nhận 20 byte.
- `G.INPUT`: đọc frame STM32 vào `D4680-D4690`.
- `M820`: báo `G.INPUT` đã hoàn thành; tín hiệu này được dùng để bắt đầu trình tự phản hồi output.

Các buffer kết thúc frame nhận được khởi tạo tại cạnh lên `X1E`:

```text
LDP X1E ──[MOV K20    U0\G164]
LDP X1E ──[MOV HFFFF  U0\G165]
```

- `U0\G164 = 20`: kết thúc một lần nhận khi đủ 20 byte.
- `U0\G165 = HFFFF`: không sử dụng CR/LF làm mã kết thúc frame.
- Nếu không dùng được `LDP X1E`, có thể dùng `X1E → PLS Mxxx`, sau đó dùng xung `Mxxx` để thực hiện hai lệnh `MOV` một lần.

Phần gửi không dùng `SM413` hoặc kích `M1` liên tục vì có thể trùng thời điểm STM32 phát. Trình tự gửi được tổ chức sau khi `M820` hoàn thành:

```text
LDP M820 ─────────────────────────[SET M900]

M900 ─────────────────────────────[T0 K1]

LDP T0 ──[X1E]──[/X0E]───────────[GP.OUTPUT U0 D4660 D4650 M810]

M810 ─────────────────────────────[RST M900]
M811 ─────────────────────────────[RST M900]
```

Nếu phiên bản GX Works không nhập trực tiếp được `LDP`, có thể tạo xung một scan bằng `PLS` hoặc dùng tiếp điểm cạnh lên tương đương.

Ý nghĩa từng phần:

| Lệnh/tín hiệu | Chức năng | Mục đích |
|---|---|---|
| `LDP M820` | Bắt cạnh lên của tín hiệu nhận hoàn tất | Chỉ tạo một yêu cầu phản hồi cho mỗi frame input. |
| `SET M900` | Giữ trạng thái đang chờ gửi | Duy trì trình tự trong thời gian timer hoạt động. |
| `T0 K1` | Tạo thời gian chờ khoảng 100 ms với timer base 100 ms | Đợi STM32 truyền xong và nhả bus trước khi PLC phát. |
| `X1E` | QJ71C24N-R4 Ready | Chỉ cho phép `GP.OUTPUT` chạy khi module sẵn sàng. |
| `/X0E` | Tiếp điểm thường đóng của CH1 error | Chặn gửi khi CH1 đang có lỗi; `X0E ON` nghĩa là có lỗi. |
| `GP.OUTPUT` | Gửi dữ liệu `D4650-D4652` | Trả sáu byte output về STM32. |
| `M810` | Hoàn thành gửi bình thường | Kết thúc chu trình và reset relay chờ. |
| `M811` | Hoàn thành gửi bất thường | Kết thúc chu trình lỗi và reset relay chờ để tránh treo trạng thái. |

Trước khi gọi `GP.OUTPUT`, các control word được chuẩn bị:

```text
MOV K1 D4660     // CH1
MOV K0 D4661     // Xóa kết quả truyền cũ
MOV K6 D4662     // Gửi 6 byte ở chế độ Byte unit
```

Các tín hiệu module được dùng khi monitor và chẩn đoán:

```text
X00 = CH1 truyền hoàn tất bình thường
X01 = CH1 truyền hoàn tất bất thường
X02 = CH1 đang truyền
X0E = CH1 có lỗi
X1E = module C24 Ready
X1F = watchdog error
```

Điều kiện vận hành bình thường:

```text
X1E = ON
X0E = OFF
X1F = OFF
M811 = OFF
D4661 = 0
```

Các thao tác `M1 OFF → ON`, `SM413`, giữ RESET STM32 và đổi mẫu `D4650` chỉ được dùng trong quá trình cô lập lỗi. Chúng không phải cơ chế kích gửi chính thức sau khi trình tự `M820 → delay → GP.OUTPUT` đã được áp dụng.

#### Mục tiêu tổng thể của lần chỉnh sửa

```text
Thay giao tiếp Modbus không phù hợp với Ladder hiện tại
→ dùng RS485 Nonprocedural raw
→ cố định frame STM32 gửi là 20 byte
→ cố định frame PLC trả về là 6 byte
→ điều khiển đúng DE/RE của SP3485E bằng PA8
→ truyền I1-I8 sang PLC và trả output PLC về O1-O8 ổn định
```

### 1. Bổ sung header giao tiếp

Trong `Core/Src/main.c`, thêm:

```c
#include "modbus_rtu.h"
```

Thay đổi này khắc phục lỗi thiếu khai báo các hàm và biến giao tiếp như `Modbus_Init()`, `Modbus_Process()`, `modbus_rx_ready` và `plc_registers`.

### 2. Chuyển luồng giao tiếp sang Nonprocedural/raw

- Thay lệnh gửi Modbus bằng `RawSerial_SendInputs(virtual_inputs, 2)`.
- STM32 gửi một gói raw cố định 20 byte để phù hợp với `G.INPUT` và `D4673 = 20` trên PLC.
- Dữ liệu được đóng gói theo thứ tự byte thấp trước để PLC lưu trực tiếp vào `D4680-D4690`.

### 3. Đọc và đóng gói I1-I8

Trong `Core/Src/main.c`:

- Đọc trạng thái GPIO của I1-I8.
- Ghép tám trạng thái vào `virtual_inputs[0]`.
- Đặt `virtual_inputs[1] = 0` cho nhóm input mở rộng.
- Gửi dữ liệu theo chu kỳ hiện tại là 600 ms.

### 4. Nhận output raw từ PLC

Trong `Core/Src/modbus_rtu.c`:

- Nhận từng byte bằng ngắt USART1.
- Xác định kết thúc gói khi đường truyền im lặng trên 5 ms.
- Chỉ xử lý gói output có đúng 6 byte từ `GP.OUTPUT`.
- Ghép 6 byte thành ba word và lưu vào `plc_registers[0..2]`.
- Bật cờ `modbus_rx_ready` khi đã nhận đủ dữ liệu.

### 5. Điều khiển O1-O8

Trong `Core/Src/main.c`:

- Sao chép `plc_registers[0]` sang `virtual_outputs[0]`.
- Dùng từng bit của `virtual_outputs[0]` để điều khiển O1-O8.
- Xóa `modbus_rx_ready` sau khi cập nhật output.

### 6. Bổ sung điều khiển hướng truyền RS485

Sau khi đối chiếu `Board_Extension_AMR_v1.pdf`, xác định IC `SP3485E` có `/RE` và `DE` nối chung với chân `PA8`.

Các thay đổi đã thực hiện:

- `Core/Inc/main.h`: khai báo `RS485_DE_Pin = GPIO_PIN_8` và port GPIOA.
- `Core/Src/gpio.c`: cấu hình PA8 là output push-pull và mặc định ở mức thấp.
- `Core/Src/modbus_rtu.c`: thêm hàm `RS485_Transmit()`.

Trình tự điều khiển:

```text
PA8 = 1 → SP3485E chuyển sang phát
UART truyền hoàn tất
PA8 = 0 → SP3485E trở về nhận
```

Thay đổi này khắc phục tình trạng STM32 giữ bus ở chế độ phát làm dữ liệu PLC trả về bị sai.

### 7. Luồng firmware sau cập nhật

```text
STM32 đọc I1-I8
→ đóng gói raw 20 byte
→ PLC nhận bằng G.INPUT tại D4680-D4690
→ PLC gửi 6 byte từ D4650-D4652 bằng GP.OUTPUT
→ STM32 nhận và ghép dữ liệu
→ cập nhật O1-O8
```

### 8. Các thay đổi trên PLC QJ71C24N-R4

#### 8.1. Cấu hình cổng truyền thông CH1

Trong Switch Setting của QJ71C24N-R4, cấu hình CH1:

```text
Operation setting       = Independent
Data bit                = 8
Parity                  = None
Stop bit                = 1
Sum check code          = None
Online change           = Disable
Setting modifications   = Disable
Communication rate      = 9600 bps
Communication protocol  = Nonprocedural protocol
```

Trong phần thiết lập dữ liệu truyền/nhận, chọn đơn vị chiều dài là `Byte`.

#### 8.2. Khởi tạo điều kiện kết thúc gói nhận

PLC được cấu hình nhận đủ 20 byte cho mỗi gói input STM32 và không chờ mã kết thúc CR/LF:

```text
U0\G164 = K20
U0\G165 = HFFFF
```

Hai giá trị này có thể được khởi tạo khi PLC bắt đầu RUN hoặc khi module báo Ready.

#### 8.3. Cấu hình lệnh nhận G.INPUT

Vùng control nhận:

```text
D4670 = 1       // Nhận trên CH1
D4671 = 0       // Kết quả nhận
D4672 = 0       // Số dữ liệu nhận, do hệ thống cập nhật
D4673 = 20      // Dung lượng vùng nhận
```

Lệnh nhận:

```text
G.INPUT U0 D4670 D4680 M820
```

Trong đó:

- `D4680-D4690` là vùng lưu input nhận từ STM32.
- `M820` là thiết bị báo hoàn thành lệnh nhận.
- Dữ liệu input chính nằm tại `D4680`.

Kết quả kiểm tra:

```text
Không kích input → D4680 = 00FFH
Kích I1          → D4680 = 00FEH
Kích I2          → D4680 = 00FDH
Kích I8          → D4680 = 007FH
```

#### 8.4. Cấu hình lệnh gửi GP.OUTPUT

Vùng control gửi:

```text
D4660 = 1       // Gửi trên CH1
D4661 = 0       // Kết quả truyền bình thường
D4662 = 6       // Số byte cần gửi
```

Vùng dữ liệu gửi:

```text
D4650 = trạng thái O1-O16
D4651 = trạng thái O17-O32
D4652 = dữ liệu dự phòng
```

Lệnh gửi:

```text
GP.OUTPUT U0 D4660 D4650 M810
```

Trong đó:

- `M810` báo hoàn thành gửi bình thường.
- `M811` báo hoàn thành bất thường.
- PLC chỉ thực hiện gửi sau khi đã nhận xong gói input, tránh hai thiết bị phát đồng thời trên bus RS485.

Ví dụ kiểm thử:

```text
D4650 = 0001H
D4651 = 0000H
D4652 = 0000H
```

Gói output quan sát được trên tool:

```text
01 00 00 00 00 00
```

#### 8.5. Kết quả sửa PLC

- QJ71C24N-R4 nhận đúng từng gói input 20 byte.
- Dữ liệu input ổn định tại đúng vùng `D4680-D4690`, không còn dịch qua các thanh ghi.
- PLC gửi đúng 6 byte output từ `D4650-D4652`.
- Đèn RD/SD của module hoạt động đúng theo chiều nhận/gửi.
- Dữ liệu PLC và dữ liệu trên tool giám sát giống nhau.

### 9. Kết quả ngày 10/08/2026

- Source build được sau khi bổ sung header giao tiếp.
- STM32 gửi đúng trạng thái I1-I8 dưới dạng gói raw 20 byte.
- PLC nhận đúng và ổn định dữ liệu input.
- PLC gửi đúng gói output 6 byte.
- STM32 nhả bus đúng sau khi truyền và nhận được phản hồi PLC.
- Tool giám sát hiển thị đúng dữ liệu hai chiều.
