# Qt Multimedia FLAC Metadata Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Select the first scanned song and read FLAC title, artist, album, and cover art with Qt Multimedia.

**Architecture:** `MusicScanner` remains the worker-thread enumerator. A scanner-local `QMediaPlayer` probes each path sequentially, then `QMediaMetaData` values supplement safe filename and color fallbacks. `MusicPlayerViewModel` owns current selection and QML remains unchanged.

**Tech Stack:** C++17, Qt 6.11 QtMultimedia, Qt Test.

## Global Constraints

- Do not parse FLAC/Vorbis/ID3 bytes in project code.
- Do not start playback or attach audio output during metadata probing.
- Preserve fallback title, artist, and generated-color art when metadata is absent.
- Keep scanner work off the GUI/QML thread.

---

### Task 1: Deliver scanned songs safely and select the first one

**Files:**

- Modify: `src/services/MusicScanner.h`
- Modify: `src/viewmodels/MusicPlayerViewModel.cpp`
- Modify: `tests/tst_music_playback.cpp`

**Interfaces:** `Q_DECLARE_METATYPE(SongData)` enables queued scanner signals; `onSongFound(const SongData&)` assigns current index zero only for the first fresh row.

- [ ] **Step 1: Write the failing regression test**

Add a test that constructs `MusicPlayerViewModel vm(nullptr, false, false)`, invokes its `onSongFound` slot with a `SongData` carrying `Qt FLAC Title` and `Qt FLAC Artist`, then expects `rowCount() == 1`, `currentIndex() == 0`, and the TitleRole value to equal `Qt FLAC Title`.

- [ ] **Step 2: Run RED**

Run `cmake --build build -j2 --target tst_music_playback && ctest --test-dir build -R tst_music_playback --output-on-failure`. Expect the new assertion to fail because the current index is `-1`.

- [ ] **Step 3: Implement the minimum fix**

Add `Q_DECLARE_METATYPE(SongData)` after the `SongData` structure. Register it in `MusicPlayerViewModel` before the scanner connections. In `onSongFound`, after the row is inserted, set `m_currentIndex` to zero and emit `currentIndexChanged()` only if the model now contains exactly one song and no valid current index exists.

- [ ] **Step 4: Run GREEN and commit**

Re-run the focused command; expect pass. Commit `MusicScanner.h`, `MusicPlayerViewModel.cpp`, and `tst_music_playback.cpp` with `fix: select first scanned music track`.

### Task 2: Map public Qt metadata into SongData

**Files:**

- Modify: `src/services/MusicScanner.h`
- Modify: `src/services/MusicScanner.cpp`
- Modify: `src/viewmodels/MusicPlayerViewModel.h`
- Modify: `src/viewmodels/MusicPlayerViewModel.cpp`
- Modify: `tests/tst_music_playback.cpp`

**Interfaces:** add `QString SongData::album`, `AlbumRole`, and public static `MusicScanner::applyQtMetaData(SongData &, const QMediaMetaData &)`.

- [ ] **Step 1: Write the failing metadata test**

Construct a fallback `SongData` and `QMediaMetaData`; insert `Title = Tag title`, `ContributingArtist = Tag artist`, and `AlbumTitle = Tag album`. Invoke `MusicScanner::applyQtMetaData`; expect those three SongData fields to equal the tags.

- [ ] **Step 2: Run RED**

Run the focused music test target. Expect compilation failure because the method and album field are absent.

- [ ] **Step 3: Implement metadata mapping and role**

Implement `applyQtMetaData` with only public Qt keys: non-empty `Title`, `ContributingArtist` (falling back to `AlbumArtist`), and `AlbumTitle` overwrite defaults. Convert a non-null `CoverArtImage` `QImage` to a PNG `data:image/png;base64,...` URI through `QBuffer`. Add `AlbumRole` exposed as model role name `album`.

- [ ] **Step 4: Run GREEN and commit**

Re-run the focused target; expect pass. Commit the five changed files with `feat: map Qt music metadata`.

### Task 3: Probe each file asynchronously through Qt Multimedia

**Files:**

- Modify: `src/services/MusicScanner.cpp`
- Modify: `tests/tst_music_playback.cpp`

**Interfaces:** `scanLibrary(const QString&)` emits each completed `SongData` after a bounded Qt metadata probe.

- [ ] **Step 1: Write the failing fallback scan test**

Create a temporary directory with a file named `Fallback Artist - Fallback Title.flac`. Connect `MusicScanner::songFound` to a `QSignalSpy`, call `scanLibrary(tempDir.path())`, and expect one emitted song retaining that title and artist when the invalid fixture produces no Qt metadata.

- [ ] **Step 2: Run RED**

Run the focused target. Expect the test to fail until the scan uses the deterministic Qt probe completion path.

- [ ] **Step 3: Implement the bounded Qt probe**

For each enumerated path, create a local `QMediaPlayer`, connect `mediaStatusChanged` to quit a local `QEventLoop` on `LoadedMedia` or `InvalidMedia`, and add a single-shot 1500 ms timeout that also quits. Set the local file source, execute the loop, then call `applyQtMetaData(song, player.metaData())`; do not call `play()` or set an audio output. Emit the finished SongData and continue the existing cancellation-aware loop.

- [ ] **Step 4: Run GREEN and commit**

Re-run the focused target; expect all music tests pass. Commit with `feat: scan FLAC metadata with Qt Multimedia`.

### Task 4: Full verification

**Files:** none unless a scoped failure requires correction.

- [ ] **Step 1: Build and test**

Run `cmake -S . -B build && cmake --build build -j2 && ctest --test-dir build --output-on-failure`. Expect successful configuration/build and all CTest targets passing.

- [ ] **Step 2: Check diff hygiene**

Run `git diff --check && git status --short`. Expect no whitespace errors and no unintended files.
