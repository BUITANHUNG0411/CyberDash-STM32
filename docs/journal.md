# 📔 Project Journal (Chronological Decision Log)

> **AI Context**: File này dùng để AI ghi chép lại các quyết định kỹ thuật quan trọng và lý do đằng sau chúng theo trình tự thời gian. Đọc file này giúp AI khôi phục lại "trí nhớ" về bối cảnh dự án.

## 2026-07-26

### Hoàn tất Phase 10 (Unit Tests) & Phase 12 (Functional Telltale Bar)
- **Vấn đề 1**: Mục cuối Phase 10 tham chiếu `updateRawTelemetry` — method không tồn tại (tên thật là `updateTelemetry`, đã có test). Lỗ hổng thật là 3 property `battery`/`range`/`temperature` thiếu test READ/WRITE/NOTIFY theo mandate của `testing_strategy.md`.
- **Quyết định 1**: Thêm `testSetBattery`/`testSetRange`/`testSetTemperature` vào `tests/main.cpp` theo pattern QSignalSpy sẵn có; sửa lại tasks board cho đúng tên method.
- **Vấn đề 2**: Top bar của `DashboardScreen` là 4 `Rectangle` trang trí giả — chỉ 1 cái gắn dữ liệu thật.
- **Quyết định 2**: Thay bằng 3 `NeonIcon` telltale gắn dữ liệu có sẵn: Warning (`isWarning`), Low Battery (`battery < 20`), High Temp (`temperature > 85` — ngưỡng tắt ở mức nền 57°C nhưng sáng khi `ErrorInjection` mô phỏng quá nhiệt). Cố tình **không** thêm xi-nhan/đèn pha vì cần property backend mới — người dùng xác nhận giữ phạm vi trong dữ liệu hiện có. Không thêm token màu mới (dùng lại `warningRed`/`textSecondary`); `NeonIcon` được thêm `Behavior on opacity` để fade mượt theo quy tắc animation của `ui_ux_guidelines.md`.

## 2026-07-17

### Cập nhật Kiến trúc Documentation (Vibe Coding Optimization)
- **Vấn đề**: Cấu trúc Markdown hiện tại tốt nhưng thiếu các cơ chế lưu trữ trí nhớ dài hạn (Memory) và luồng công việc (Workflows) rõ ràng cho AI.
- **Quyết định**: 
  - Khởi tạo file `journal.md` này để giữ lịch sử quyết định (Chronological log).
  - Giữ nguyên cấu trúc Metadata `> **AI Context**:` vì Gemini 3.1 Pro đọc cực kỳ tốt mà không tốn token như YAML.
  - Tạo thư mục `.agents/workflows/` để chứa các quy trình nhiều bước, bắt đầu bằng `brainstorming.md`.
  - Ban hành `DOCUMENTATION_STANDARDS.md` để quy chuẩn hóa cách viết Markdown (sử dụng GFM Alerts `> [!WARNING]`, định danh code blocks rõ ràng).
