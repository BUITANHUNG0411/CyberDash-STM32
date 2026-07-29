# Qt Creator Baseline Runtime Diagnosis Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Identify the last Qt Creator launch that produced a usable dashboard, restore that exact source/binary baseline without losing the current Parking Assist work, and isolate the first runtime regression before implementing a minimal fix.

**Architecture:** Keep the current working tree intact while comparing clean historical worktrees. The investigation separates compilation, QML root creation, Qt Creator launch configuration, and Multimedia/graphics runtime; no host GPU or driver state is changed. A code fix is permitted only after one reproducible failing baseline and one known-good comparison are recorded.

**Tech Stack:** Git worktrees, CMake/Ninja, Qt 6.11, Qt Quick/QML, Qt Test, Qt Creator Desktop Debug kit.

**Status:** Paused after the independent `CenterHub.qml:32` QML compile error was fixed. Resume this plan only if Qt Creator still fails after rebuilding the corrected QML source.

## Global Constraints

- Preserve the AGENTS.md Zero-JavaScript QML, C++17, MVVM, and hardware-boundary rules.
- Do not install packages, change AMD/NVIDIA drivers, edit `/etc`, or alter host GPU configuration.
- Do not reset or discard the current worktree; isolate historical comparisons under `/tmp`.
- Do not change Parking Assist/CenterHub behavior while diagnosing the launch regression.
- Every candidate must pass configure, build, CTest, QML policy scan, and an offscreen smoke before it is called a baseline.

---

### Task 1: Record the clean working-tree boundary

**Files:**
- Read: `AGENTS.md`, `docs/tasks_board.md`, `docs/architecture.md`, `docs/testing_strategy.md`
- Modify: `docs/journal.md`

- [ ] **Step 1: Capture the current state without mutating it**

Run:

```bash
git status --short
git log --oneline --decorate -12
git diff -- src/main.cpp src/viewmodels/MusicPlayerViewModel.cpp src/viewmodels/MusicPlayerViewModel.h
```

Expected: the current VDPAU compatibility experiment is either present and documented or absent as one isolated patch; Parking Assist/CenterHub files remain as the current uncommitted feature boundary. Do not reset either state while this plan is paused.

- [ ] **Step 2: Record rollback evidence**

Append a journal entry stating that the VDPAU/lazy-player experiment was removed, the targeted source files match `HEAD`, and the current feature files were preserved. Do not claim the Qt Creator GUI is fixed until a user-side Qt Creator launch succeeds.

- [ ] **Step 3: Verify documentation consistency**

Run:

```bash
rg -n "VDPAU|QT_FATAL_WARNINGS|QT_FFMPEG|lazy-player|research_vdpau" README.md docs src qml || true
```

Expected: no active project documentation claims that the rejected VDPAU workaround is implemented.

### Task 2: Validate the last known pre-Parking baseline in isolation

**Files:**
- Create temporarily: `/tmp/cyberdash-baseline-4307a4d/`
- Read: historical commit `4307a4d`

- [ ] **Step 1: Create a detached historical worktree**

Run:

```bash
rm -rf /tmp/cyberdash-baseline-4307a4d
git worktree add --detach /tmp/cyberdash-baseline-4307a4d 4307a4d
```

Expected: a clean tree at `4307a4d` (`fix: close final baseline review gaps`) with no changes to the current worktree.

- [ ] **Step 2: Build and test the historical candidate**

Run inside the temporary worktree:

```bash
cmake -S . -B build -G Ninja
cmake --build build -j2
ctest --test-dir build --output-on-failure
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
set +e
QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator >/tmp/cyberdash-4307a4d-smoke.log 2>&1
printf 'smoke_exit=%s\n' "$?"
set -e
```

Expected: configure/build/tests/lint pass; smoke exit is `124` and contains no QML object-creation failure. Any different result is recorded as evidence that the issue is environment-wide rather than introduced by Parking Assist.

- [ ] **Step 3: Remove only the temporary worktree**

Run:

```bash
git worktree remove --force /tmp/cyberdash-baseline-4307a4d
```

Expected: the current worktree is unchanged.

### Task 3: Bisect the runtime regression boundary

**Files:**
- Read-only comparison: commits `4307a4d`, `a587775`, `99c38a3`, `06e6295`, `156e75a`, `17f4b55`
- Read-only comparison: `src/main.cpp`, `qml/Main.qml`, `qml/screens/DashboardScreen.qml`, `qml/components/CenterHub.qml`, `qml/components/ParkingAssistView.qml`, `CMakeLists.txt`

- [ ] **Step 1: Compare only runtime-relevant changes**

Run:

```bash
git diff --stat 4307a4d..17f4b55 -- src qml CMakeLists.txt
git log --oneline --reverse 4307a4d..17f4b55 -- src qml CMakeLists.txt
```

Expected: a short ordered list of runtime changes, separated from documentation-only commits.

- [ ] **Step 2: Test each candidate in a clean worktree**

For each candidate (`4307a4d`, `a587775`, `99c38a3`, `06e6295`, `156e75a`, `17f4b55`), repeat Task 2 Step 2 and record a table with commit, binary path, offscreen result, Qt Creator result, and first error line. Do not modify candidate files.

- [ ] **Step 3: Select the first failing boundary**

Choose the earliest commit whose binary fails under the same launch mode. The selected boundary must have one preceding passing candidate and one failing candidate; otherwise stop and classify the failure as environment/toolchain-wide instead of changing application code.

### Task 4: Create a minimal fix plan from evidence

**Files:**
- Modify only the file(s) implicated by Task 3
- Test: the nearest existing C++ or QML test target for that component

- [ ] **Step 1: Write a focused failing test or reproducible launch command**

Use a C++ Qt Test for ViewModel/service behavior. For graphics/QML startup, use the exact historical binary and Qt Creator run configuration as the reproducible fixture; do not add driver-dependent tests.

- [ ] **Step 2: Implement one root-cause fix**

Keep the fix limited to the first failing boundary. Do not combine Multimedia changes, QML layout changes, and environment changes in one patch.

- [ ] **Step 3: Verify the fix against both candidates**

Run configure, full build, CTest, Zero-JS scan, QML review, and offscreen smoke. Then rebuild the exact `build/Desktop_Debug/QtStmAutomotiveSimulator` path used by Qt Creator and ask for one fresh Application Output capture.

### Task 5: Update project records and hand off safely

**Files:**
- Modify: `docs/journal.md`, `docs/tasks_board.md`, and the relevant architecture/testing document

- [ ] **Step 1: Record the proven boundary and evidence**

Document the passing commit, failing commit, exact command, exit code, and the minimal fix. Do not document hypotheses as root causes.

- [ ] **Step 2: Run the final repository checks**

Run:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
git diff --check
```

- [ ] **Step 3: Stop before commit until the user confirms Qt Creator launch**

Do not commit or push the diagnostic/fix patch until the user confirms that the exact Qt Creator target launches the dashboard. If it still fails, preserve the new Application Output and continue from the selected historical boundary rather than trying another unrelated workaround.
