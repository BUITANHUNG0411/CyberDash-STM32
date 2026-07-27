---
name: qt-cpp-docs
description: >-
  Generates standalone Markdown reference documentation for any Qt/C++ source
  files in the Automotive UI Showcase (Qt 6 / QML + STM32) project. Documents MVVM 
  architecture, thread-safety, QML exposure, and UART handling.
license: LicenseRef-Qt-Commercial OR BSD-3-Clause
compatibility: >-
  Designed for Claude Code, GitHub Copilot, and similar agents.
metadata:
  author: qt-ai-skills
  version: "1.0"
  qt-version: "6.x"
  category: process
---

# Qt C++ Documentation Skill

You are an expert in Qt/C++ who writes clear, accurate Markdown documentation for any C++ source file in a Qt project. Generate structured reference docs (`.md` format, NOT QDoc) summarizing how the file fits into the project.

## Core requirements
- **No code fences** except for a Usage Example section. Use inline code for method signatures (e.g. `#### void setFilePath(const QString &path)`).
- **Header is truth.** Trust `.h` files for public API definitions, and use `.cpp` for behavioral context.
- **Tables for properties.** Document `Q_PROPERTY` declarations and enums using clean markdown tables.
- **Access-level discipline.** Document `public` API fully, `protected` separately, and silently skip `private` unless exposed via QML.

## Document structure
For each C++ class, generate a `<ClassName>.md` file in the `doc/` subdirectory containing these applicable sections:

1. **Class Overview**: Brief summary of the class's purpose and role in the architecture.
2. **Project Structure and Dependencies**: Qt modules, `#include` relationships, and build requirements.
3. **Class Hierarchy**: Explain inherited traits from base classes (e.g., `QObject` signals/slots).
4. **Q_PROPERTY Declarations**: Table listing Property, Type, READ, WRITE, NOTIFY, and Description.
5. **Enumerations (Q_ENUM / Q_FLAG)**: Table listing Value, Integer, and Description.
6. **Public Member Variables**: Table listing Type and Description.
7. **Signals**: List each signal with its parameters and trigger conditions.
8. **Public Slots and Q_INVOKABLE Methods**: List each method, its purpose, side-effects, and QML availability.
9. **Public Methods**: General API methods and thread-safety constraints.
10. **Protected Virtual Methods**: Key overrides (e.g., `paintEvent`).
11. **Ownership and Lifecycle**: Mention `QObject` parenting, memory management, and `deleteLater()` constraints.
12. **Thread Safety**: Specify GUI-thread only, thread-safe, or single-threaded limits.
13. **QML Exposure**: Name the registered QML type and accessible interfaces.
14. **Inter-Class Interactions**: Connections to other models or services.
15. **External Communication**: Network, IPC, D-Bus, or Serial hardware logic (e.g., STM32 UART handling).
16. **Usage Example**: A minimal C++ snippet for reusable classes.

## Project Custom Rules (Automotive UI Showcase)

### 1. Document MVVM ViewModel Contract
Every `Q_PROPERTY` and `Q_INVOKABLE` must document:
- The **vehicle mode** it feeds.
- The **property binding flow** to QML.
- The **thread of operation** (must be GUI thread).

### 2. SimulatorService / SerialService Convergence
Explicitly document how `SimulatorService` (QTimer-based full-dashboard telemetry)
and `SerialService` (QSerialPort-based raw wire telemetry) converge at the
`VehicleStatusViewModel` boundary through `main.cpp` and `TelemetryMapper`; they
do not expose one shared full-dashboard signal.
- Note **thread affinity**: `SerialService` performs bounded, non-blocking
  `readyRead` parsing on the GUI thread. Only `MusicScanner` is required to run
  off-thread.
- Note **UART frame format**: newline-delimited `TEL` ASCII records such as
  `TEL,118,11.8,0;129\n`, 115200 8N1, toward STM32F103C8T6.

### 3. Memory and Lifecycle Annotations
For `QTimer` or `QSerialPort` instances:
- State owner and teardown sequence.
- Note re-entrancy guards for callbacks.

## Pre-flight: Existing Documentation
Before writing docs, check if `doc/<FileName>.md` already exists. If it does, pause and Ask the User whether to Update, Skip, Generate Fresh, or Cancel.

## Entry Point (main.cpp) Structure
If documenting `main.cpp`, structure the document differently:
A. Overview
B. Qt Application Setup
C. Command-Line Handling
D. Top-Level Object Creation
E. Wiring and Connections
F. Event Loop
G. Dependencies

Do not output quality checklists or "AI assistance" tags. Produce purely authoritative developer reference documentation.
