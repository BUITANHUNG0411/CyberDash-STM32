# Music Player UI and Backend Design

> **AI Context**: Active design and implementation contract for the MusicPlayer QML view, MusicPlayerViewModel, and asynchronous MusicScanner pipeline.

> [!IMPORTANT]
> QML remains a passive view. Playback, scanning, scrubber state, normalization, and seek requests belong to the C++ ViewModel/service layer.

## Understanding Summary
- **What is being built:** A Music Player UI component (`MusicPlayer.qml`) for the QtStmAutomotiveSimulator.
- **Why it exists:** To provide multimedia playback visualization inside the automotive dashboard.
- **Who it is for:** The driver/user interacting with the digital instrument cluster.
- **Key constraints:** Strict adherence to the "Neon Cyberpunk" aesthetic, zero JavaScript logic in QML, pure declarative bindings.
- **Current boundary:** `MusicPlayerViewModel` owns playback and scrubber behavior; `MusicScanner` performs library discovery on its worker thread. QML only binds state and forwards direct invokable calls.

## Assumptions
- **Performance:** 3D Cover Flow animation relies on QML `PathView` with property bindings (`scale`, `z`, `opacity`) to ensure 60 FPS on embedded targets without the overhead of a true 3D engine.
- **Resource Loading:** Cover images are loaded asynchronously (`asynchronous: true`) to avoid UI stuttering.

## Decision Log
- **Layout Approach:** Decided to use `PathView` for a 3D Cover Flow instead of hardcoded explicit States.
  - *Alternatives considered:* Hardcoded 3-Item Layout using QML States.
  - *Why chosen:* Provides a native, fluid carousel animation, is scalable if more items are added, and completely avoids complex JS or State transition logic.
- **Control Panel Integration:** Separated into a distinct Glassmorphism panel below the cover art.
  - *Alternatives considered:* Controls floating directly over the cover art.
  - *Why chosen:* Better UX for automotive environments, allowing a wider progress bar and larger touch targets.
- **Neon Aesthetic:** Decided to use static Cyberpunk theme colors (`Theme.primaryColor` / `Theme.accentColor`).
  - *Alternatives considered:* Dynamic colors extracting dominant colors from the current cover art.
  - *Why chosen:* Maintains absolute consistency with the existing dashboard gauges (e.g., NeonTickGauge).

## Final Design Breakdown
### 1. Cover Flow Area (fills remaining height)
- **Component:** `PathView` with a custom `Path`.
- **Delegate:** Rounded `Image` (via `OpacityMask`) with a static Neon glow shadow.
- **Animation:** `PathAttribute` drives `scale` (1.0 at center, 0.7 at sides), `opacity` (1.0 at center, 0.5 at sides), and `z` index for the 3D depth illusion.

### 2. Navigation Control Panel (fixed 176px)
- **Container:** Glassmorphism `Rectangle` with translucent background and subtle borders. Height is sized to the centered `Column` content (track info + scrubber + volume + controls) plus ~16px of top/bottom breathing room — a shorter panel visually crushes the Play/Pause button against the bottom edge.
- **Track Info:** Two `Text` elements (Title and Artist) bound to the active item in the `PathView`.
- **Progress Bar:** Thin neon line with a glowing thumb.
- **Media Controls:** `RowLayout` centering Previous, Play/Pause, and Next vector icons with scale-down and neon-flare effects on press.

---

## Backend C++ Specification

### Current implementation
- **What is being built:** A C++ backend (`MusicScanner` and `MusicPlayerViewModel`) for the music player.
- **Why it exists:** To scan local `.mp3`/`.flac` files and feed real data to the UI without blocking the main thread.
- **Who it is for:** Users interacting with the Qt Automotive Dashboard.
- **Key constraints:** Zero JS in QML, strict MVVM architecture, non-blocking UI (threaded), and the repository C++17 standard.
- **Out of scope:** Network streaming, full media-management workflows, and complex playlist authoring.

### Assumptions
- The default scanning path is the OS standard `Music` directory (`QStandardPaths::MusicLocation`).
- Qt Multimedia (`QMediaPlayer` / `QMediaMetaData`) is sufficient for extracting Title and Artist, avoiding external dependencies like TagLib.
- Missing album art will fall back to static colors to maintain the Neon Cyberpunk aesthetic.

### Decision Log
- **Scanning Trigger Strategy:** Manual Scan via UI Button.
  - *Alternatives considered:* Auto-scan on startup.
  - *Why chosen:* Gives user control and prevents unwanted disk I/O spikes when the application boots up, typical in embedded systems.
- **Threading Model:** `QThread` with a Worker Object (`MusicScanner`).
  - *Alternatives considered:* `QtConcurrent::run` returning a batch `QList<SongData>`.
  - *Why chosen:* Allows real-time row insertion (fluid UI updates) as each file is scanned, rather than waiting for the entire disk traversal to complete. Industry standard for non-blocking I/O.

### Final Design Breakdown
#### 1. Data Structure (`SongData`)
- Simple C++ struct containing `QString title`, `artist`, `filePath`, and mock `color1`, `color2` for fallback UI.

#### 2. `MusicScanner` (Worker)
- Inherits `QObject` and is moved to a background `QThread`.
- Contains `scanLibrary(QString path)` slot.
- Uses `QDirIterator` to recursively find audio files.
- Emits `songFound(SongData)` upon successful metadata extraction.
- Emits `scanFinished()` when complete or if the directory is empty.

#### 3. `MusicPlayerViewModel` (Model/Controller)
- Inherits `QAbstractListModel` to provide a declarative backend for the QML `PathView`.
- Manages the lifecycle of `MusicScanner` and its `QThread`.
- Captures `songFound` to execute `beginInsertRows`/`endInsertRows` for dynamic, lag-free UI updates.
- Exposes standard player controls via `Q_INVOKABLE`: `play()`, `pause()`, `next()`, `prev()`, `scanLibrary()`.
- Implements safety flags (`m_isScanning`) to prevent concurrent scan spamming.
- Ensures safe thread termination (`quit()`, `wait()`) in its destructor.

#### 4. Testing Strategy
- Create a mock directory with dummy `.mp3` files in `tests/`.
- Verify `MusicScanner` signal emission counts via QTest.

## Troubleshooting

- **Cover flow is empty:** confirm `MusicScanner` completed on its worker thread and that `MusicPlayerViewModel` received `songFound` before the model is rendered.
- **Playback controls do nothing:** verify the QML handler forwards a direct `MusicViewModel` invokable and that the ViewModel exposes a valid current media source.
- **Scrubbing jumps or seeks repeatedly:** keep pointer state and normalization in `MusicPlayerViewModel`; QML should only forward coordinates and lifecycle calls.
- **The UI freezes during a scan:** ensure `QDirIterator` runs only in `MusicScanner`'s worker thread and that model row insertion returns to the GUI thread.
