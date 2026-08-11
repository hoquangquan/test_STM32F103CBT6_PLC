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
