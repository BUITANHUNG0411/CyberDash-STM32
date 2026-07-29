# English Documentation Hygiene Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep all tracked Markdown in English and synchronize active project documentation with the current ten-ViewModel, six-test implementation.

**Architecture:** Documentation-only changes preserve historical records while translating their prose. Active specifications and the README are updated from the checked-in CMake, `main.cpp`, ViewModel, and QML contracts; no production behavior changes.

**Tech Stack:** Markdown, CMake project metadata, Qt 6 application architecture.

## Global Constraints

- Do not delete assets, duplicated agent routers, archived plans/specs, or research records without a separate owner decision.
- Preserve dated historical claims as historical evidence; do not rewrite their old verification counts as current results.
- Keep `AGENTS.md` and `CLAUDE.md` synchronized.
- All edited Markdown prose must be English.

---

### Task 1: Translate existing Vietnamese Markdown prose

**Files:**
- Modify: `.agents/workflows/brainstorming.md`
- Modify: `docs/journal.md`

- [x] Translate active workflow metadata and historical journal prose faithfully, preserving SOP requirements, dates, code identifiers, and historical tense.
- [x] Scan tracked Markdown for Vietnamese characters and confirm no remaining prose matches.

### Task 2: Synchronize active documentation with current implementation

**Files:**
- Modify: `README.md`
- Modify: `AGENTS.md`
- Modify: `CLAUDE.md`
- Modify: `docs/architecture.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/tasks_board.md`

- [x] Update ten-ViewModel and six-CTest facts, context tables, paths, architecture flow, verification coverage, and phases 26–27.
- [x] Document the passive Cockpit Context Rail and preserve existing Serial, Safety, and Parking contracts.
- [x] Confirm both router files remain byte-identical after their synchronized update.

### Task 3: Verify documentation hygiene

**Files:**
- Verify only: all Git-tracked Markdown and the existing build/test matrix.

- [x] Run the tracked-Markdown English scan, diff whitespace check, and full CTest suite.
- [x] Review the final diff for unintended source or asset changes.
