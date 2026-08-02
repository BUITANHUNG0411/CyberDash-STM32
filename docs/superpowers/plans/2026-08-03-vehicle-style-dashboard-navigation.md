# Vehicle-Style Dashboard Navigation Implementation Plan

> For agentic workers: use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

Goal: Add a vehicle-like central navigation flow that moves between Music, Parking Assist, and Trip Computer with C++-owned page state and keyboard controls that emulate left/right steering-wheel buttons.

Architecture: Extend the existing CenterHubViewModel, which already owns CenterHub page selection and the critical parking-safety override, instead of introducing a second navigation state machine. Add a passive TripComputerView.qml page, expose it through the existing StackLayout, and bind Ctrl+Left/Ctrl+Right shortcuts directly to C++ invokables from the window shell.

Tech Stack: Qt 6.8+, C++17, Qt Quick/QML, Qt Test, CMake.

## Global Constraints

- QML remains passive: no executable JavaScript, local mutation, loops, or calculations.
- CenterHubViewModel remains the single source of truth for CenterHub page selection.
- A live critical rear-parking sample always prevents navigation away from Parking Assist.
- Existing Music/Parking swipe and mouse-tab interactions remain supported.
- The visual system continues to use Theme.qml tokens and the Double Arch dashboard geometry.
- Keyboard shortcuts are a PC surrogate for real vehicle left/right controls; no STM32 protocol change is included.
- All new ViewModel APIs run on the GUI thread and emit notifications only when effective state changes.
- TripComputerViewModel rejects non-finite speed samples and normalizes non-positive maxDeltaMs at its C++ boundary.
- CenterHubViewModel keeps the ParkingAssist dependency non-owning but lifetime-safe through QPointer.
- Do not commit changes, per user request.

## File Map

- Modify src/viewmodels/CenterHubViewModel.h: add the Trip page, page count/label properties, and directional navigation API.
- Modify src/viewmodels/CenterHubViewModel.cpp: validate the three-page range, implement bounded directional movement, format the active page label, and centralize the parking override.
- Modify src/viewmodels/TripComputerViewModel.h/.cpp: expose a C++-formatted average-speed display string, validate time/speed inputs, and suppress redundant notifications.
- Modify tests/main.cpp and tests/tst_parking_assist.cpp: add red/green tests for Trip display hardening, page metadata, directional movement, boundary behavior, and safety locking.
- Create qml/components/TripComputerView.qml: render Trip/ODO/average-speed data from TripComputer without owning business logic.
- Modify qml/components/CenterHub.qml: add the Trip page and third page indicator while preserving the existing swipe handler.
- Modify qml/Main.qml: add direct Ctrl+Left and Ctrl+Right shortcuts to the C++ navigation API.
- Modify CMakeLists.txt: register TripComputerView.qml in the existing Qt QML module.
- Do not modify the pre-existing user change in AGENTS.md.

## Decision Log

| Decision | Alternatives considered | Reason |
|---|---|---|
| Extend CenterHubViewModel | Add a separate DashboardNavigationViewModel | The existing VM already owns page state, swipe thresholds, and parking safety precedence; a second owner would create synchronization risk. |
| Add Trip as the third CenterHub page | Add only keyboard shortcuts for the two existing pages | Keyboard-only navigation would duplicate the current two-page capability without a new destination. Trip is already a retained product concern and is visible in the dashboard footer. |
| Use bounded directional movement | Wrap around at the first/last page | Bounded movement is predictable for a driver-style control and avoids jumping from Trip back to Music unexpectedly. |
| Keep page changes immediate | Add a separate focus/confirm menu state | The dashboard already presents stable page content and page tabs; immediate movement keeps the first slice small while still providing OEM-like directional control. |
| Keep critical Parking Assist precedence | Allow manual navigation to override the warning | Safety behavior is an immutable project decision and must remain stronger than manual input. |

---

### Task 1: Extend the C++ navigation contract with tests first

Files:
- Modify tests/tst_parking_assist.cpp
- Modify src/viewmodels/CenterHubViewModel.h
- Modify src/viewmodels/CenterHubViewModel.cpp

Interfaces:
- Produces CenterHubViewModel::TripPage = 2.
- Produces int pageCount() const, returning 3.
- Produces QString activePageLabel() const, returning MUSIC, PARK, or TRIP.
- Produces bool moveSelection(int direction), accepting only -1 or 1; it returns false for invalid directions or a blocked/boundary move.

- [ ] Step 1: Write the failing tests

Add these test slots to TestParkingAssist in tests/tst_parking_assist.cpp:

    void centerHubExposesVehicleStylePageNavigation()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);

        QCOMPARE(hub.pageCount(), 3);
        QCOMPARE(hub.activePageLabel(), QStringLiteral("MUSIC"));

        QVERIFY(hub.moveSelection(1));
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        QCOMPARE(hub.activePageLabel(), QStringLiteral("PARK"));

        QVERIFY(hub.moveSelection(1));
        QCOMPARE(hub.activePage(), CenterHubViewModel::TripPage);
        QCOMPARE(hub.activePageLabel(), QStringLiteral("TRIP"));
    }

    void centerHubDirectionalNavigationStopsAtBoundaries()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);

        QVERIFY(!hub.moveSelection(-1));
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);

        QVERIFY(hub.selectPage(CenterHubViewModel::TripPage));
        QVERIFY(!hub.moveSelection(1));
        QCOMPARE(hub.activePage(), CenterHubViewModel::TripPage);
        QVERIFY(!hub.moveSelection(0));
        QVERIFY(!hub.moveSelection(2));
    }

    void centerHubCriticalParkingLocksDirectionalNavigation()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);

        parking.updateSensorSample(29, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        QVERIFY(!hub.moveSelection(-1));
        QVERIFY(!hub.moveSelection(1));
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
    }

Update the old invalid-page test so Trip is valid and only 3 and larger remain invalid:

    void centerHubRejectsInvalidPageRequest()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);

        QVERIFY(hub.selectPage(CenterHubViewModel::TripPage));
        QCOMPARE(hub.activePage(), CenterHubViewModel::TripPage);
        QVERIFY(!hub.selectPage(3));
        QCOMPARE(hub.activePage(), CenterHubViewModel::TripPage);
    }

- [ ] Step 2: Run the focused test to verify it fails for the missing API

Run:

    cmake --build build --target tst_parking_assist -j2

Expected: compilation failure naming the missing TripPage, pageCount, activePageLabel, and moveSelection members. This confirms the tests exercise the new contract rather than passing against existing behavior.

- [ ] Step 3: Add the minimal C++ API

In src/viewmodels/CenterHubViewModel.h, replace the two-value enum and add the new declarations:

    #include <QObject>
    #include <QString>

    Q_PROPERTY(int activePage READ activePage NOTIFY activePageChanged)
    Q_PROPERTY(int pageCount READ pageCount CONSTANT)
    Q_PROPERTY(QString activePageLabel READ activePageLabel NOTIFY activePageChanged)

    enum Page : int {
        MusicPage = 0,
        ParkingPage = 1,
        TripPage = 2,
    };
    Q_ENUM(Page)

    int pageCount() const;
    QString activePageLabel() const;

    Q_INVOKABLE bool moveSelection(int direction);

In src/viewmodels/CenterHubViewModel.cpp, add:

    constexpr int firstPage = CenterHubViewModel::MusicPage;
    constexpr int lastPage = CenterHubViewModel::TripPage;

Implement the new methods and update validation:

    int CenterHubViewModel::pageCount() const
    {
        return lastPage - firstPage + 1;
    }

    QString CenterHubViewModel::activePageLabel() const
    {
        switch (m_activePage) {
        case MusicPage:
            return QStringLiteral("MUSIC");
        case ParkingPage:
            return QStringLiteral("PARK");
        case TripPage:
            return QStringLiteral("TRIP");
        default:
            return QStringLiteral("MUSIC");
        }
    }

    bool CenterHubViewModel::moveSelection(int direction)
    {
        if (direction != -1 && direction != 1) {
            return false;
        }

        const int candidatePage = m_requestedPage + direction;
        if (candidatePage < firstPage || candidatePage > lastPage) {
            return false;
        }

        return selectPage(candidatePage);
    }

Change selectPage validation to:

    if (page < firstPage || page > lastPage) {
        return false;
    }

Keep the existing critical-proximity rejection unchanged, so Music and Trip cannot displace a live critical Parking page.

- [ ] Step 4: Run the focused test to verify it passes

Run:

    cmake --build build --target tst_parking_assist -j2
    ctest --test-dir build -R tst_parking_assist --output-on-failure

Expected: the target builds and tst_parking_assist passes, including all existing parking/safety tests.

### Task 2: Add the Trip Computer CenterHub page

Files:
- Modify src/viewmodels/TripComputerViewModel.h
- Modify src/viewmodels/TripComputerViewModel.cpp
- Create qml/components/TripComputerView.qml
- Modify qml/components/CenterHub.qml
- Modify CMakeLists.txt

Interfaces:
- Consumes TripComputer.tripDisplay, TripComputer.odoDisplay, and TripComputer.avgSpeedDisplay.
- Consumes CenterHubController.activePage for selection/indicator bindings.
- Produces a passive QML page with no JavaScript and no local business state.

- [ ] Step 1: Add the C++-formatted average-speed property

Add to src/viewmodels/TripComputerViewModel.h:

    Q_PROPERTY(QString avgSpeedDisplay READ avgSpeedDisplay NOTIFY tripChanged)

    QString avgSpeedDisplay() const;

Implement in src/viewmodels/TripComputerViewModel.cpp:

    QString TripComputerViewModel::avgSpeedDisplay() const
    {
        return QString::number(avgSpeedKmh(), 'f', 1) + QStringLiteral(" km/h");
    }

This keeps all formatting in C++ and follows the existing tripDisplay/odoDisplay contract.

While touching this ViewModel, keep its input boundary safe: normalize maxDeltaMs to at least 1 ms, reject non-finite speedKmh values before distance math, ignore non-positive clamped deltas, and emit tripChanged only when odometer, trip distance, or average speed actually changes. resetTrip must be a no-op when both trip distance and elapsed trip time are already zero.

- [ ] Step 2: Create the passive Trip Computer view

Create qml/components/TripComputerView.qml:

    import QtQuick
    import QtQuick.Layouts
    import com.showcase

    GlassPanel {
        id: root

        ColumnLayout {
            anchors {
                fill: parent
                leftMargin: Theme.spaceXl
                rightMargin: Theme.spaceXl
                topMargin: Theme.spaceXl
                bottomMargin: Theme.spaceXl
            }
            spacing: Theme.spaceLg

            Text {
                text: "TRIP COMPUTER"
                color: Theme.accentCyan
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textMd
                    bold: true
                    letterSpacing: 2
                }
                Layout.alignment: Qt.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: Theme.spaceLg

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: Theme.spaceSm

                    Text {
                        text: "TRIP"
                        color: Theme.textSecondary
                        font { family: Theme.fontMain; pixelSize: Theme.textSm; letterSpacing: 1 }
                        Layout.alignment: Qt.AlignHCenter
                    }
                    GlowingText {
                        text: TripComputer.tripDisplay
                        color: Theme.textPrimary
                        glowColor: Theme.accentCyan
                        font.pixelSize: Theme.displayMd
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: Theme.spaceSm

                    Text {
                        text: "ODO"
                        color: Theme.textSecondary
                        font { family: Theme.fontMain; pixelSize: Theme.textSm; letterSpacing: 1 }
                        Layout.alignment: Qt.AlignHCenter
                    }
                    GlowingText {
                        text: TripComputer.odoDisplay
                        color: Theme.textPrimary
                        glowColor: Theme.accentCyan
                        font.pixelSize: Theme.displayMd
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: Theme.spaceSm

                    Text {
                        text: "AVG SPEED"
                        color: Theme.textSecondary
                        font { family: Theme.fontMain; pixelSize: Theme.textSm; letterSpacing: 1 }
                        Layout.alignment: Qt.AlignHCenter
                    }
                    GlowingText {
                        text: TripComputer.avgSpeedDisplay
                        color: Theme.textPrimary
                        glowColor: Theme.accentCyan
                        font.pixelSize: Theme.displayMd
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            Text {
                text: "CTRL + LEFT / RIGHT TO NAVIGATE"
                color: Theme.textSecondary
                font { family: Theme.fontMain; pixelSize: Theme.textXs; letterSpacing: 1 }
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

- [ ] Step 3: Add Trip to the stable CenterHub StackLayout and indicators

In qml/components/CenterHub.qml, add the third page after ParkingAssistView:

        TripComputerView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

Add a third fixed indicator matching the existing MUSIC/PARK indicators. It must use:

    color: CenterHubController.activePage === 2 ? Theme.accentCyan : Theme.trackInactive

Its label is TRIP and its click handler is:

    onClicked: CenterHubController.selectPage(2)

Do not add a QML model or delegate for this fixed three-item navigation strip.

- [ ] Step 4: Register the new QML file

Add this entry to the QML_FILES list in CMakeLists.txt:

        qml/components/TripComputerView.qml

- [ ] Step 5: Run the QML compile/build check

Run:

    cmake -S . -B build
    cmake --build build --target QtStmAutomotiveSimulator -j2

Expected: configuration and application build complete successfully with the new QML component registered in com.showcase.

### Task 3: Add car-style PC controls without changing the UART boundary

Files:
- Modify qml/Main.qml

Interfaces:
- Consumes CenterHubController.moveSelection(int).
- Produces two window-scoped Shortcut bindings; no QML state or JavaScript logic.

- [ ] Step 1: Add direct keyboard shortcut calls

Insert after the existing Escape shortcut in qml/Main.qml:

    Shortcut {
        sequence: "Ctrl+Left"
        onActivated: CenterHubController.moveSelection(-1)
    }

    Shortcut {
        sequence: "Ctrl+Right"
        onActivated: CenterHubController.moveSelection(1)
    }

- [ ] Step 2: Scan the changed QML for forbidden executable JavaScript

Run:

    rg -n '\b(function|if|for|while|switch|var|let|const|Number|toFixed)\b' qml/Main.qml qml/components/CenterHub.qml qml/components/TripComputerView.qml

Expected: no matches. The only handlers are direct calls to C++ invokables.

### Task 4: Full verification and project review

Files:
- Verify all changed C++/QML/CMake files.
- Do not commit.

- [ ] Step 1: Run the full registered test suite

    ctest --test-dir build --output-on-failure

Expected: all registered tests pass.

- [ ] Step 2: Run the full configure and build checks

    cmake -S . -B build
    cmake --build build -j2

Expected: both commands exit with status 0.

- [ ] Step 3: Run the QML review workflow

Use the project qt-qml-review skill on qml/Main.qml, qml/components/CenterHub.qml, and qml/components/TripComputerView.qml. Confirm zero-JavaScript compliance, valid bindings, stable layout usage, and no new recursive MultiEffect source.

- [ ] Step 4: Run the C++ review workflow

Use the project qt-cpp-review skill on src/viewmodels/CenterHubViewModel.* and src/viewmodels/TripComputerViewModel.*. Confirm GUI-thread ownership, notification behavior, enum/API consistency, and no regression to parking safety precedence.

- [ ] Step 5: Inspect the final diff and preserve the user's pre-existing change

    git diff --check
    git status --short
    git diff --stat

Expected: AGENTS.md remains modified as it was before this task; new modifications are limited to the files in this plan. No commit or push is performed.

## Plan Self-Review

- Scope is one cohesive subsystem: central dashboard navigation plus its Trip destination.
- Existing Music/Parking swipe, mouse selection, and critical parking override remain covered by the current test suite.
- All new public names are defined before they are consumed: TripPage, pageCount, activePageLabel, moveSelection, and avgSpeedDisplay.
- All average-speed formatting is in C++; QML contains only the avgSpeedDisplay binding.
- No new UART, service, vehicle mode, persistence, or external dependency is introduced.
- The user explicitly requested no commit, so no commit or push is part of execution.
