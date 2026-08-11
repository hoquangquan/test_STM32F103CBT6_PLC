# Sổ Tay Các Lệnh Git Cơ Bản (Git Cheat Sheet)

Dưới đây là tổng hợp các lệnh Git từ cơ bản đến thực chiến nhất, sắp xếp theo quy trình làm việc để bạn dễ tra cứu trong quá trình làm dự án STM32.

---

## 1. Quy trình kết nối lần đầu (Đưa code từ máy lên GitHub)
*(Bạn chỉ cần làm bước này duy nhất 1 lần khi tạo kho (Repository) mới trên GitHub)*

```bash
git init                                     # Khởi tạo Git cho thư mục hiện tại
git remote add origin <Link_GitHub_của_bạn>  # Kết nối thư mục hiện tại với kho trên mạng
git push -u origin master                    # Đẩy code lần đầu tiên lên nhánh chính (master)
```

---

## 2. Quy trình code hàng ngày (Sửa code xong -> Lưu lại -> Đẩy lên)
*(Đây là vòng lặp bạn sẽ dùng nhiều nhất. Cứ mỗi lần code xong một tính năng và chạy OK là gõ 3 lệnh này)*

```bash
git add .                                    # Gom TẤT CẢ các file vừa sửa vào danh sách chờ
git commit -m "Ghi chú những gì bạn vừa sửa" # Chốt (Lưu) thành 1 phiên bản cố định
git push origin master                       # Đẩy phiên bản đó lên GitHub
```

> [!TIP]
> Bạn nên commit thường xuyên với những ghi chú (message) ngắn gọn nhưng rõ ràng (Ví dụ: "Sửa lỗi không nhận Modbus", "Thêm cấu hình chân PA7"). Việc này giúp bạn dễ dàng khôi phục lại code nếu sau này lỡ tay làm hỏng.

---

## 3. Quy trình tải code về (Đồng bộ code từ máy khác)
*(Dùng khi bạn đổi máy tính khác, hoặc lấy code đồng nghiệp vừa đẩy lên)*

```bash
git clone <Link_GitHub>                      # Tải TOÀN BỘ dự án mới tinh về máy (Chỉ dùng khi máy chưa có dự án)
git pull origin master                       # Kéo code MỚI NHẤT từ trên mạng về gộp vào thư mục hiện tại
```

> [!IMPORTANT]
> **Quy tắc vàng:** Trước khi bắt đầu gõ dòng code nào ở bất kỳ máy tính nào, việc **đầu tiên** bạn phải làm là mở Terminal và gõ lệnh `git pull origin master` để lấy code mới nhất về. Nếu không làm vậy, bạn sẽ bị lỗi đụng độ (Conflict) khi đẩy code lên!

---

## 4. Các lệnh kiểm tra (Bắt bệnh - Debug)

```bash
git status                                   # Xem file nào vừa bị sửa, file nào chưa được add (Rất hay dùng)
git log --oneline                            # Xem lịch sử các lần bạn đã commit
git remote -v                                # Kiểm tra xem dự án đang được link với địa chỉ GitHub nào
```
