# Hướng dẫn Git cho dự án STM32

Repository GitHub:

```text
https://github.com/hoquangquan/test_STM32F103CBT6_PLC
```

Thư mục dự án trên máy:

```text
C:\Users\ESATECH\Downloads\test_STM32F103CBT6_PLC
```

## 1. Kiểm tra Git và repository

```powershell
git --version
cd "C:\Users\ESATECH\Downloads\test_STM32F103CBT6_PLC"
git status
git branch --show-current
git remote -v
```

Trạng thái bình thường:

```text
On branch main
Your branch is up to date with 'origin/main'.
nothing to commit, working tree clean
```

## 2. Tải dự án lần đầu trên máy khác

Chỉ dùng `clone` khi máy chưa có repository:

```powershell
cd "C:\Users\<TEN_NGUOI_DUNG>\Downloads"
git clone "https://github.com/hoquangquan/test_STM32F103CBT6_PLC.git"
cd .\test_STM32F103CBT6_PLC
git status
```

Không chạy `git clone` bên trong repository đã clone vì sẽ tạo repository lồng nhau.

## 3. Đồng bộ code trước khi sửa

```powershell
git switch main
git fetch origin
git pull --ff-only origin main
git status
```

`--ff-only` giúp Git dừng lại nếu lịch sử hai bên đã phân nhánh, tránh tự tạo merge commit ngoài ý muốn.

## 4. Tạo nhánh mới

Không nên sửa trực tiếp trên `main`. Sau khi đồng bộ `main`, tạo nhánh cho công việc mới:

```powershell
git switch -c update/ten-cong-viec
git branch --show-current
```

Ví dụ:

```powershell
git switch -c update/rs485-plc-output
```

Tên nhánh nên viết không dấu, không có khoảng trắng và mô tả ngắn gọn công việc.

## 5. Kiểm tra và commit thay đổi

```powershell
git status
git diff
git diff --stat
```

Chỉ thêm các file cần thiết:

```powershell
git add Core/Src/main.c
git add Core/Src/modbus_rtu.c
git add Core/Src/gpio.c
git add Core/Inc/main.h
git add Core/Inc/modbus_rtu.h
git add TESTING_GUIDE.md
git add GIT_GUIDE.md
```

Hoặc thêm tất cả sau khi đã kiểm tra kỹ:

```powershell
git add .
```

Kiểm tra nội dung đã đưa vào vùng chờ commit:

```powershell
git diff --cached
git status
```

Tạo commit:

```powershell
git commit -m "Update RS485 PLC I-O communication"
```

## 6. Push nhánh lên GitHub

Lần đầu push nhánh mới:

```powershell
git push -u origin update/ten-cong-viec
```

Những lần tiếp theo trên cùng nhánh:

```powershell
git push
```

Sau đó mở GitHub và tạo Pull Request:

- `base`: `main`
- `compare`: `update/ten-cong-viec`

Kiểm tra tab **Files changed** trước khi merge.

## 7. Kiểm tra xung đột

Tải trạng thái mới nhất và xem khác biệt:

```powershell
git fetch origin
git status
git diff --name-status origin/main...HEAD
git log --oneline --graph --decorate --all -15
```

Thử hợp nhất để kiểm tra:

```powershell
git merge --no-commit --no-ff origin/main
```

Nếu chỉ kiểm tra và chưa muốn giữ kết quả merge:

```powershell
git merge --abort
```

Xem các file đang xung đột:

```powershell
git status
git diff --name-only --diff-filter=U
```

Trong file xung đột sẽ có dạng:

```text
<<<<<<< HEAD
Nội dung nhánh hiện tại
=======
Nội dung từ nhánh được hợp nhất
>>>>>>> origin/main
```

Sửa file, xóa các dấu trên, giữ nội dung đúng rồi hoàn tất:

```powershell
git add <TEN_FILE_DA_SUA>
git commit -m "Resolve merge conflicts"
```

Trên GitHub:

- `Able to merge`: không có xung đột.
- `Can't automatically merge`: có xung đột cần xử lý.

## 8. Cập nhật sau khi Pull Request đã merge

```powershell
git switch main
git pull --ff-only origin main
```

Nếu không cần nhánh cũ nữa, kiểm tra đã merge rồi mới xóa:

```powershell
git branch --merged main
git branch -d update/ten-cong-viec
git push origin --delete update/ten-cong-viec
```

## 9. Các lệnh kiểm tra thường dùng

```powershell
git status                         # Trạng thái file và nhánh
git branch -a                      # Tất cả nhánh local và remote
git branch -vv                     # Nhánh và upstream tương ứng
git log --oneline -10              # 10 commit gần nhất
git log --oneline --graph --all    # Sơ đồ lịch sử các nhánh
git remote -v                      # Địa chỉ repository GitHub
git diff                           # Thay đổi chưa được add
git diff --cached                  # Thay đổi đã được add
git diff --name-only               # Chỉ xem tên file thay đổi
```

## 10. Quy tắc an toàn

- Luôn chạy `git status` trước và sau thao tác quan trọng.
- Đồng bộ `main` trước khi tạo nhánh mới.
- Không đưa thư mục build như `Debug/` lên GitHub.
- Không chạy `git init` nếu dự án đã có `.git`.
- Không tạo repository Git lồng trong repository khác.
- Không xóa `.git` hoặc dùng `git reset --hard` khi chưa hiểu rõ hậu quả.
- Không push trực tiếp lên `main` khi đang phát triển; dùng nhánh và Pull Request.
- Commit nhỏ, rõ mục đích và chỉ chứa file liên quan.

## 11. Quy trình hằng ngày

```powershell
cd "C:\Users\ESATECH\Downloads\test_STM32F103CBT6_PLC"
git switch main
git pull --ff-only origin main
git switch -c update/ten-cong-viec

# Sửa code và kiểm tra trên thiết bị

git status
git diff
git add .
git diff --cached
git commit -m "Mo ta noi dung da sua"
git push -u origin update/ten-cong-viec
```

Sau khi push, tạo Pull Request trên GitHub và chỉ merge khi đã kiểm tra **Files changed** và không còn xung đột.
