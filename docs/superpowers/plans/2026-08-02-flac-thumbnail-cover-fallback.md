# FLAC Thumbnail Cover Fallback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Display embedded FLAC cover art when the Qt backend exposes it through `ThumbnailImage` while preserving title, artist, album, and existing fallbacks.

**Architecture:** Keep `MusicScanner` as the Qt metadata mapping boundary. Prefer `QMediaMetaData::CoverArtImage`, fall back to `QMediaMetaData::ThumbnailImage`, encode the selected `QImage` through the existing PNG data-URI path, and leave `SongData::coverArt` unchanged when neither key is usable.

**Tech Stack:** C++17, Qt 6 Core/Gui/Multimedia, Qt Test, CMake/CTest.

## Global Constraints

- Do not add TagLib, FFmpeg APIs, or any external library.
- Do not parse FLAC metadata blocks or Vorbis comments in project code.
- Do not modify files under `/home/buitanhung0411/Music/`.
- Keep directory scanning and metadata probing off the QML thread.
- Keep QML passive and free of imperative JavaScript.
- Preserve filename, existing cover, and generated-gradient fallbacks.

---

### Task 1: Map Qt thumbnail metadata to the existing cover role

**Files:**

- Modify: `tests/tst_music_playback.cpp`
- Modify: `src/services/MusicScanner.cpp`

**Interfaces:**

- Consumes: `static void MusicScanner::applyQtMetaData(SongData &, const QMediaMetaData &)` and `QString SongData::coverArt`.
- Produces: the same interface, with deterministic `CoverArtImage` → `ThumbnailImage` → existing-value precedence.

- [ ] **Step 1: Write failing image-key regressions**

Add `#include <QImage>` and three focused Qt Test slots. Generate 1×1 RGB images in memory so tests do not read the user's Music directory:

```cpp
void qt_thumbnail_image_becomes_cover_art_test()
{
    SongData song;
    QImage thumbnail(1, 1, QImage::Format_RGB32);
    thumbnail.fill(Qt::cyan);
    QMediaMetaData metadata;
    metadata.insert(QMediaMetaData::ThumbnailImage, thumbnail);

    MusicScanner::applyQtMetaData(song, metadata);

    QVERIFY(song.coverArt.startsWith(QStringLiteral("data:image/png;base64,")));
}

void qt_cover_art_image_takes_precedence_test()
{
    SongData song;
    QImage cover(1, 1, QImage::Format_RGB32);
    cover.fill(Qt::red);
    QImage thumbnail(1, 1, QImage::Format_RGB32);
    thumbnail.fill(Qt::blue);
    QMediaMetaData preferred;
    preferred.insert(QMediaMetaData::CoverArtImage, cover);
    QMediaMetaData fallback;
    fallback.insert(QMediaMetaData::ThumbnailImage, thumbnail);
    QMediaMetaData both = fallback;
    both.insert(QMediaMetaData::CoverArtImage, cover);

    SongData expected;
    MusicScanner::applyQtMetaData(expected, preferred);
    MusicScanner::applyQtMetaData(song, both);

    QCOMPARE(song.coverArt, expected.coverArt);
}

void qt_missing_images_preserve_cover_fallback_test()
{
    SongData song;
    song.coverArt = QStringLiteral("fallback-cover");
    QMediaMetaData metadata;

    MusicScanner::applyQtMetaData(song, metadata);

    QCOMPARE(song.coverArt, QStringLiteral("fallback-cover"));
}
```

- [ ] **Step 2: Run RED**

Run:

```bash
cmake --build build -j2 --target tst_music_playback
ctest --test-dir build -R tst_music_playback --output-on-failure
```

Expected: `qt_thumbnail_image_becomes_cover_art_test` fails because the current mapper reads only `CoverArtImage`; the precedence and missing-image tests pass.

- [ ] **Step 3: Implement the minimum metadata fallback**

Replace the single-key image lookup in `MusicScanner::applyQtMetaData` with:

```cpp
QImage coverArt = metadata.value(QMediaMetaData::CoverArtImage).value<QImage>();
if (coverArt.isNull())
    coverArt = metadata.value(QMediaMetaData::ThumbnailImage).value<QImage>();

if (!coverArt.isNull()) {
    QByteArray encodedCover;
    QBuffer buffer(&encodedCover);
    if (buffer.open(QIODevice::WriteOnly) && coverArt.save(&buffer, "PNG"))
        song.coverArt = "data:image/png;base64," + QString::fromLatin1(encodedCover.toBase64());
}
```

- [ ] **Step 4: Run GREEN**

Run:

```bash
cmake --build build -j2 --target tst_music_playback
ctest --test-dir build -R tst_music_playback --output-on-failure
```

Expected: focused target builds and `tst_music_playback` passes.

- [ ] **Step 5: Commit the focused fix**

```bash
git add tests/tst_music_playback.cpp src/services/MusicScanner.cpp
git commit -m "fix: read FLAC thumbnail cover metadata"
```

### Task 2: Verify the complete repository contract

**Files:** No production changes unless a failure is directly caused by Task 1.

**Interfaces:**

- Consumes: the updated `MusicScanner::applyQtMetaData` behavior from Task 1.
- Produces: fresh build, test, QML-policy, runtime, and diff-hygiene evidence.

- [ ] **Step 1: Configure, build, and run all registered tests**

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Expected: configuration and build succeed; every registered CTest passes.

- [ ] **Step 2: Run the Zero-JavaScript and project QML checks**

```bash
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml -g '*.qml'
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
```

Expected: the policy scan has no executable-JavaScript matches; project QML lint passes.

- [ ] **Step 3: Smoke the application**

```bash
QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator
```

Expected: exit `124` from the timeout is acceptable; output contains no QML/runtime error. Host audio or GPU-backend warnings are environmental and must be reported rather than treated as application failures.

- [ ] **Step 4: Check scoped diff hygiene**

```bash
git diff --check -- src/services/MusicScanner.cpp tests/tst_music_playback.cpp
git status --short
```

Expected: no whitespace errors; status shows no unintended files from this task.

