# Music Track Information Layout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the scanned song title and artist visible by removing the circular QML width dependency in the Music control panel.

**Architecture:** Keep the verified C++ metadata pipeline unchanged. Give the QML track-information column explicit bounds from the row's left edge to the Scan button's left edge, and let both text items consume that bounded width.

**Tech Stack:** Qt 6.11 Quick/QML, C++17 Qt Test source-contract regression, CMake/CTest.

## Global Constraints

- Do not change `MusicScanner` or `MusicPlayerViewModel` behavior.
- Do not add external libraries.
- Keep QML declarative with zero imperative JavaScript.
- Preserve the Scan button size, title/artist bindings, centered alignment, and elision.
- Preserve all unrelated user changes in the dirty worktree.

---

### Task 1: Bound the track information area

**Files:**

- Modify: `tests/tst_music_playback.cpp`
- Modify: `qml/components/MusicPlayer.qml`

**Interfaces:**

- Consumes: `MusicViewModel.currentTitle`, `MusicViewModel.currentArtist`, and the existing 60-pixel Scan button.
- Produces: QML IDs `trackInfo` and `scanButton`, with `trackInfo` bounded to the left of `scanButton` and child text widths equal to `trackInfo.width`.

- [ ] **Step 1: Write the failing QML source-contract test**

Add `#include <QFile>` and this Qt Test slot to `TestMusicPlayback`:

```cpp
void track_info_has_explicit_width_without_scan_overlap_test()
{
    const QString path = QFINDTESTDATA("../qml/components/MusicPlayer.qml");
    QVERIFY2(!path.isEmpty(), "MusicPlayer.qml test data was not found");
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString qml = QString::fromUtf8(file.readAll());

    QVERIFY(qml.contains(QStringLiteral("id: trackInfo")));
    QVERIFY(qml.contains(QStringLiteral("anchors.left: parent.left")));
    QVERIFY(qml.contains(QStringLiteral("anchors.right: scanButton.left")));
    QVERIFY(qml.contains(QStringLiteral("anchors.rightMargin: Theme.spaceMd")));
    QVERIFY(qml.contains(QStringLiteral("id: scanButton")));
    QVERIFY(!qml.contains(QStringLiteral("width: parent.width - 80")));
}
```

- [ ] **Step 2: Run RED**

```bash
cmake --build build -j2 --target tst_music_playback
ctest --test-dir build -R tst_music_playback --output-on-failure
```

Expected: the new test fails at `id: trackInfo` because the current QML has no bounded track-info item and still contains `width: parent.width - 80`.

- [ ] **Step 3: Implement the minimum declarative layout fix**

Change the track-information column and Scan button in `MusicPlayer.qml` to:

```qml
Column {
    id: trackInfo
    anchors.left: parent.left
    anchors.right: scanButton.left
    anchors.rightMargin: Theme.spaceMd
    anchors.verticalCenter: parent.verticalCenter
    spacing: 2

    Text {
        text: MusicViewModel.currentTitle !== "" ? MusicViewModel.currentTitle : (MusicViewModel.isScanning ? "Scanning..." : "No Music")
        font.family: Theme.fontMain
        font.pixelSize: Theme.textMd
        font.bold: true
        color: Theme.textPrimary
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }
    Text {
        text: MusicViewModel.currentArtist
        font.family: Theme.fontMain
        font.pixelSize: Theme.textXs
        color: Theme.textSecondary
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }
}

Rectangle {
    id: scanButton
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter
    width: 60
```

- [ ] **Step 4: Run GREEN**

```bash
cmake --build build -j2 --target tst_music_playback
ctest --test-dir build -R tst_music_playback --output-on-failure
```

Expected: `tst_music_playback` passes.

- [ ] **Step 5: Run focused QML checks**

```bash
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py qml/components/MusicPlayer.qml
cmake --build build --target QtStmAutomotiveSimulator_qmllint
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml/components/MusicPlayer.qml
```

Expected: project lint passes; module `qmllint` produces no new warning caused by the changed block; Zero-JavaScript matches are comment-only.

- [ ] **Step 6: Commit the focused fix**

```bash
git add tests/tst_music_playback.cpp qml/components/MusicPlayer.qml
git commit -m "fix: show current music track information"
```

### Task 2: Verify the repository

**Files:** No changes unless a failure is directly caused by Task 1.

**Interfaces:**

- Consumes: the bounded QML track-information layout from Task 1.
- Produces: fresh build, test, policy, smoke, and diff-hygiene evidence.

- [ ] **Step 1: Run the full build and test suite**

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Expected: build succeeds and all registered tests pass.

- [ ] **Step 2: Run repository QML policy checks**

```bash
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml -g '*.qml'
rg --files qml -g '*.qml' | python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py --files-from=-
```

Expected: Zero-JavaScript matches are comment-only and project lint passes.

- [ ] **Step 3: Smoke and check diff hygiene**

```bash
QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator
git diff --check HEAD~1..HEAD
git status --short
```

Expected: smoke reaches timeout `124` without a QML/runtime error; diff check is clean; status contains no unintended task artifacts.
