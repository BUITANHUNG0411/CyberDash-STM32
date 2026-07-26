# 📔 Project Journal (Chronological Decision Log)

> **AI Context**: File này dùng để AI ghi chép lại các quyết định kỹ thuật quan trọng và lý do đằng sau chúng theo trình tự thời gian. Đọc file này giúp AI khôi phục lại "trí nhớ" về bối cảnh dự án.

## 2026-07-26

### Phase 14: Vehicle Morphing (Bike / Scooter / Car)
- **Vấn đề**: Trụ cột "morphs across form factors" trong Product Vision chưa hề được implement — chỉ có một layout Car cố định, repo chưa dùng QML `States`/`Transitions` bao giờ.
- **Quyết định 1 (Kiến trúc)**: `VehicleModeViewModel` riêng (context property `VehicleMode`, string `vehicleMode`, `cycleVehicleMode()` car→bike→scooter→car) — đúng Decision Log "QML States bound to C++ string properties" và pattern one-VM-per-concern. UART frame không có trường vehicle-type nên state thuần UI; nút cycle disable khi boot (`enabled: !ThemeController.isBooting`) để sweep không đua với đổi maxValue.
- **Quyết định 2 (Morph trong khung bất biến)**: Bezel Double Arch giữ nguyên tuyệt đối; chỉ morph nội dung. State `"car"` RỖNG — base bindings chính là layout Car, nên khởi động pixel-identical. `PropertyChanges` (cú pháp classic) tự khôi phục binding gốc khi rời state — nhờ đó override `bottomBar.opacity` ở Bike không phá binding boot.
- **Quyết định 3 (Một gauge, rebind theo state)**: Vòm phải dùng MỘT `NeonTickGauge` đổi `value: vm.rpm ↔ vm.battery` + maxValue/tickCount qua state, thay vì 2 gauge chồng cross-fade (tránh 2 MultiEffect bloom chạy song song). Khoảnh khắc Repeater relabel tick được che bằng transition "dip": fade+scale 2 vòm về 0 → `PropertyAction` áp swap khi vô hình → nổi lại OutBack. Chỉ animate opacity/scale (không width/height) đúng chuẩn QML.
- **Quyết định 4 (Center swap)**: `MusicPlayer` KHÔNG đặt trong Loader (subtree nặng nhất; unload sẽ reset PathView giữa lúc phát — audio thuộc C++ nên chỉ ẩn bằng opacity + `visible: opacity > 0` là đủ, MultiEffect ngừng render khi invisible). Card Scooter (`RangeTripCard.qml`) nạp qua Loader async với trick tự unload Zero-JS: `active: VehicleMode.vehicleMode === "scooter" || opacity > 0`.
- **Bug tiềm ẩn tìm thấy**: `EnergyBlocks` là Item không có implicit size (0×0) — vô hình khi đặt trong `Column`. Fix: `implicitWidth/Height` từ Row bên trong; nhờ đó cả bottom bar lẫn card scooter hiển thị đúng dải pin.
- Spec/Plan: `/home/buitanhung0411/.claude/plans/` (Phase 14); screenshots per-mode xác nhận bezel bất biến ở cả 3 mode, 2 theme.
- **Vấn đề**: Cluster chỉ có một theme tối cố định và khởi động "bật thẳng" — thiếu chiều sâu automotive (self-test, gauge sweep) và không có chế độ ban ngày.
- **Quyết định 1 (Kiến trúc)**: Tạo `ThemeViewModel` riêng (context property `ThemeController`) thay vì nhét vào `VehicleStatusViewModel` — UI chrome tách khỏi telemetry, tái dùng được cho Drive Modes/Vehicle Morphing sau này. Boot timeline chạy hoàn toàn trong C++ (`QSequentialAnimationGroup` + 2 `QVariantAnimation` legs, InOutQuad 900ms/chiều); constructor nhận duration để test chạy ~10ms với `QSignalSpy`/`QTRY_COMPARE_WITH_TIMEOUT`.
- **Quyết định 2 (Theme)**: Token màu trong `Theme.qml` thành ternary theo `ThemeController.isNight`, kèm `Behavior { ColorAnimation }` đặt tập trung ngay trong singleton — toàn app cross-fade mà không sửa component nào. Token themed phải bỏ `readonly` (Behavior không gắn được vào readonly property). Bezel Double Arch giữ nguyên cả 2 theme qua token hằng mới `bezelStroke` (tách khỏi `textSecondary` vốn giờ đổi theo theme); fill của Shape là "mặt màn hình" nên vẫn đổi sáng/tối.
- **Quyết định 3 (Boot)**: QML chỉ bind ternary vào `bootStage`/`isBooting`/`bootProgress`: telltale sáng hết khi self-test, `NeonTickGauge.displayedValue` hiển thị sweep (Behavior `enabled: !isBooting` để bám sát timeline C++), center panel + bottom bar fade-in ở stage 2. `startBootSequence()` idempotent, gọi sau `engine.loadFromModule`.
- Spec: `docs/superpowers/specs/2026-07-26-day-night-theme-startup-animation-design.md`; Plan: `docs/superpowers/plans/2026-07-26-day-night-theme-startup-animation.md`.
- **Bug tìm thấy khi user test (đã fix)**: `blurBackdrop` trong `MusicPlayer.qml` dùng `MultiEffect { source: parent }` — source chứa chính effect → ShaderEffectSource đệ quy. Màn hình tĩnh thì vô hại, nhưng khi theme toggle chạy `ColorAnimation` 600ms, backdrop re-render liên tục làm blur tích lũy (feedback loop) thành mảng hồng che cả 2 gauge và **đóng băng vĩnh viễn** sau khi animation dừng. Fix: `backdropSource` thành hidden source (`visible: false`), `MultiEffect { source: backdropSource }` — đúng pattern sibling như `NeonIcon`/`NeonTickGauge`. **Quy tắc rút ra: KHÔNG BAO GIỜ để `MultiEffect.source` trỏ vào item chứa chính nó.** Đồng thời chuyển nút toggle theme vào trong `topBar` RowLayout để cụm 4 icon tự căn giữa (trước đó anchor ngoài làm lệch phải).

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
