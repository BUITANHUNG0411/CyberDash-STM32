# Portfolio README Refresh Implementation Plan

> For agentic workers: use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

Goal: Replace the long technical README with an English, recruiter-facing project showcase while preserving a fast path to run the Qt dashboard and links to detailed documentation.

Architecture: README.md becomes a one-page landing document. Detailed engineering material remains canonical in docs/, so the README uses short proof points and direct links instead of duplicating architecture, protocol, test, roadmap, and contribution content. The dashboard image is refreshed only if an actual screenshot capture succeeds.

Tech Stack: GitHub Flavored Markdown, CMake, Qt 6.8, existing PNG preview asset.

## Global Constraints

- Write in concise English for portfolio/recruiter readers.
- Keep the dashboard screenshot directly below the project introduction.
- Retain only verifiable claims from the repository and recent CTest evidence.
- Keep one copy-paste Quick Start block with configure, build, and run commands.
- Link detailed documentation instead of duplicating it.
- Do not introduce badges, GIFs, external hosting, fictional metrics, or claims about physical hardware validation.
- Attempt a real screenshot capture after a successful local build; preserve the existing preview if capture is unavailable or invalid.
- Do not commit or modify the pre-existing AGENTS.md change.

---

### Task 1: Rewrite README as a showcase landing page

Files:
- Modify README.md

Interfaces:
- Consumes resources/media/dashboard-preview.png.
- Links to docs/architecture.md, docs/hardware_integration.md, docs/testing_strategy.md, and docs/tasks_board.md.
- Produces a single English portfolio README with no duplicated protocol/reference manual.

- [ ] Step 1: Replace the opening with product outcome and visual proof

Use this exact top-level structure:

    # QtStmAutomotiveSimulator

    > A Qt 6 automotive dashboard that visualizes STM32 UART telemetry and stays usable through an in-process simulator fallback.

    ![Neon cyberpunk automotive dashboard](resources/media/dashboard-preview.png)

- [ ] Step 2: Add concise project proof points

Use exactly four bullets under ## What it demonstrates:

    - C++17 + Qt Quick/QML dashboard built with a strict MVVM boundary.
    - UART telemetry pipeline for STM32F103C8T6, with parser, mapper, watchdog, and automatic simulator fallback.
    - Driver-focused interactions: Music, rear Parking Assist, Trip Computer, day/night theme, and vehicle-style CenterHub navigation.
    - Passive QML: presentation only; application state and interaction logic remain in C++.

- [ ] Step 3: Add the Quick Start and proof sections

Include one shell block under ## Quick Start:

    cmake -S . -B build
    cmake --build build -j2
    ./build/QtStmAutomotiveSimulator

Include a ## Verification section stating:

    Four deterministic CTest targets cover ViewModels, music playback, serial telemetry, and Parking Assist.

Then include:

    ctest --test-dir build --output-on-failure

- [ ] Step 4: Keep a short status and documentation index

Under ## Status, state that the software telemetry pipeline and simulator fallback are tested, while physical STM32/USB-TTL field validation remains pending.

Under ## Learn more, include a four-item list linking architecture, hardware integration, testing strategy, and the current task board.

- [ ] Step 5: Remove long-form material

Remove the project tree, architecture diagram, full test-target table, UART wire contract, UI-system explanation, phase roadmap, contribution rules, and license placeholder. Those details remain in docs/ or source comments.

### Task 2: Build and capture a current dashboard preview

Files:
- Potentially modify resources/media/dashboard-preview.png only after successful capture.

Interfaces:
- Consumes the QtStmAutomotiveSimulator executable built in build/.
- Produces a valid PNG capture matching the current dashboard if the desktop capture environment is available.

- [ ] Step 1: Configure and build

Run:

    cmake -S . -B build
    cmake --build build -j2

Expected: both commands exit with status 0.

- [ ] Step 2: Attempt a time-bounded application launch

Run the executable with the available display backend. Capture a current window/root screenshot with ImageMagick import only if the process starts and the capture produces a non-empty PNG with a dashboard-sized resolution.

- [ ] Step 3: Validate before replacing the tracked preview

Inspect the candidate image dimensions and visually inspect it. Only then replace resources/media/dashboard-preview.png through an explicit, user-authorized file patch/update. If display capture is not supported, leave the current PNG untouched and report that limitation.

### Task 3: Validate the documentation handoff

Files:
- Verify README.md and resources/media/dashboard-preview.png.

- [ ] Step 1: Check Markdown references

Run:

    rg -n 'docs/|resources/media/dashboard-preview.png' README.md

Expected: every linked local target exists.

- [ ] Step 2: Inspect the final diff

Run:

    git diff --check
    git status --short

Expected: README.md and the plan are modified; the preview PNG appears only if a verified capture replaced it. AGENTS.md remains an unrelated unstaged modification.

## Plan Self-Review

- The plan removes duplication without hiding the product outcome, proof of engineering quality, or run instructions.
- Every retained claim is supported by the repository or the recent test suite.
- Screenshot replacement is conditional on a real and validated capture, preventing a blank/headless image from replacing the existing asset.
- No commit, push, or external upload is authorized.
