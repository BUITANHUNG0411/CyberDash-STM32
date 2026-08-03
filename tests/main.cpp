#include <QtTest>
#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>
#include <limits>
#include "viewmodels/VehicleStatusViewModel.h"
#include "viewmodels/ThemeViewModel.h"
#include "viewmodels/TripComputerViewModel.h"

class TestViewModels : public QObject
{
    Q_OBJECT

public:
    TestViewModels() {}
    ~TestViewModels() {}

private slots:
    void testApplicationGraphIsCarOnly() {
        const QString cmakePath = QFINDTESTDATA("../CMakeLists.txt");
        const QString mainPath = QFINDTESTDATA("../src/main.cpp");
        const QString dashboardPath = QFINDTESTDATA("../qml/screens/DashboardScreen.qml");
        QVERIFY(!cmakePath.isEmpty());
        QVERIFY(!mainPath.isEmpty());
        QVERIFY(!dashboardPath.isEmpty());

        QFile cmakeFile(cmakePath);
        QFile mainFile(mainPath);
        QFile dashboardFile(dashboardPath);
        QVERIFY(cmakeFile.open(QIODevice::ReadOnly | QIODevice::Text));
        QVERIFY(mainFile.open(QIODevice::ReadOnly | QIODevice::Text));
        QVERIFY(dashboardFile.open(QIODevice::ReadOnly | QIODevice::Text));

        const QString cmake = QString::fromUtf8(cmakeFile.readAll());
        const QString main = QString::fromUtf8(mainFile.readAll());
        const QString dashboard = QString::fromUtf8(dashboardFile.readAll());
        QVERIFY2(!cmake.contains("VehicleModeViewModel"), "VehicleMode must not be registered in CMake");
        QVERIFY2(!cmake.contains("SafetyScenario"), "Safety Lab must not be registered in CMake");
        QVERIFY2(!cmake.contains("CockpitContext"), "Context Rail must not be registered in CMake");
        QVERIFY2(!cmake.contains("VehicleDiagnostics"), "Diagnostics must not be registered in CMake");
        QVERIFY2(!cmake.contains("DriveMode"), "Drive Mode must not be registered in CMake");
        QVERIFY2(!main.contains("VehicleMode"), "VehicleMode must not be constructed or exposed");
        QVERIFY2(!main.contains("SafetyScenario"), "Safety Lab must not be constructed or exposed");
        QVERIFY2(!main.contains("CockpitContext"), "Context Rail must not be constructed or exposed");
        QVERIFY2(!main.contains("VehicleDiagnostics"), "Diagnostics must not be constructed or exposed");
        QVERIFY2(!main.contains("DriveMode"), "Drive Mode must not be constructed or exposed");
        QVERIFY2(!main.contains("bootStage"), "Boot choreography must not be exposed");
        QVERIFY2(!main.contains("bootProgress"), "Boot choreography must not be exposed");
        QVERIFY2(!dashboard.contains("VehicleMode"), "Dashboard must not contain vehicle-mode branching");
        QVERIFY2(!dashboard.contains("RangeTripCard"), "Dashboard must not load Scooter content");
        QVERIFY2(!dashboard.contains("SafetyScenario"), "Dashboard must not contain Safety Lab UI");
        QVERIFY2(!dashboard.contains("CockpitContext"), "Dashboard must not contain Context Rail UI");
        QVERIFY2(!dashboard.contains("VehicleDiagnostics"), "Dashboard must not contain Diagnostics UI");
        QVERIFY2(!dashboard.contains("DriveMode"), "Dashboard must not contain Drive Mode UI");
    }

    void testTripComputerVisualContract() {
        const QString tripPath = QFINDTESTDATA("../qml/components/TripComputerView.qml");
        const QString parkingPath = QFINDTESTDATA("../qml/components/ParkingAssistView.qml");
        QVERIFY(!tripPath.isEmpty());
        QVERIFY(!parkingPath.isEmpty());

        QFile tripFile(tripPath);
        QFile parkingFile(parkingPath);
        QVERIFY(tripFile.open(QIODevice::ReadOnly | QIODevice::Text));
        QVERIFY(parkingFile.open(QIODevice::ReadOnly | QIODevice::Text));

        const QString trip = QString::fromUtf8(tripFile.readAll());
        const QString parking = QString::fromUtf8(parkingFile.readAll());

        QVERIFY2(trip.contains(QStringLiteral("id: tripHero")),
                 "Trip must expose a dedicated hero value");
        QVERIFY2(trip.contains(QStringLiteral("Theme.tripHeroDisplay")),
                 "Trip hero size must come from Theme.qml");
        QVERIFY2(trip.contains(QStringLiteral("id: statsRow")),
                 "Trip must group secondary values in a stats row");
        QVERIFY2(trip.contains(QStringLiteral("Theme.tripMetricDisplay")),
                 "Trip stat size must come from Theme.qml");
        QVERIFY2(trip.contains(QStringLiteral("text: \"RESET TRIP\"")),
                 "Trip must expose an explicit reset affordance");
        QVERIFY2(trip.contains(QStringLiteral("TripComputer.resetTrip()")),
                 "Reset must remain a direct C++ invokable call");
        QVERIFY2(parking.contains(QStringLiteral("Theme.parkingDistanceDisplay")),
                 "Parking distance must use a dedicated compact display token");
        QVERIFY2(!parking.contains(QStringLiteral("font.pixelSize: Theme.displayMd")),
                 "Parking must not use the oversized shared display token");

        const QRegularExpression executableJs(
            QStringLiteral("\\b(function|if|for|while|switch|var|let|const)\\b"));
        QVERIFY2(!executableJs.match(trip).hasMatch(),
                 "TripComputerView.qml must contain no executable JavaScript keywords");
    }

    void testOemClusterSkinContract() {
        const QString cmakePath = QFINDTESTDATA("../CMakeLists.txt");
        const QString themePath = QFINDTESTDATA("../qml/Theme.qml");
        const QString framePath = QFINDTESTDATA("../qml/components/InstrumentFrame.qml");
        const QString glassPath = QFINDTESTDATA("../qml/components/GlassPanel.qml");
        const QString musicPath = QFINDTESTDATA("../qml/components/MusicPlayer.qml");
        const QString parkingPath = QFINDTESTDATA("../qml/components/ParkingAssistView.qml");
        const QString centerPath = QFINDTESTDATA("../qml/components/CenterHub.qml");
        QVERIFY(!cmakePath.isEmpty());
        QVERIFY(!themePath.isEmpty());
        QVERIFY(!framePath.isEmpty());
        QVERIFY(!glassPath.isEmpty());
        QVERIFY(!musicPath.isEmpty());
        QVERIFY(!parkingPath.isEmpty());
        QVERIFY(!centerPath.isEmpty());

        auto readSource = [](const QString &path) {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return QString();
            }
            return QString::fromUtf8(file.readAll());
        };

        const QString cmake = readSource(cmakePath);
        const QString theme = readSource(themePath);
        const QString frame = readSource(framePath);
        const QString glass = readSource(glassPath);
        const QString music = readSource(musicPath);
        const QString parking = readSource(parkingPath);
        const QString center = readSource(centerPath);

        QVERIFY2(cmake.contains(QStringLiteral("qml/components/InstrumentFrame.qml")),
                 "InstrumentFrame must be registered in the QML module");
        QVERIFY2(theme.contains(QStringLiteral("clusterFrameLineWidth: 2")),
                 "The OEM frame must remain visually legible at dashboard scale");
        QVERIFY2(theme.contains(QStringLiteral("clusterFrameHighlightRatio: 0.48")),
                 "The OEM accent highlight must be long enough to read as a bezel");
        QVERIFY2(theme.contains(QStringLiteral("centerStatusLedWidth: 32")),
                 "CenterHub status LEDs must be visible at the dashboard scale");
        QVERIFY2(frame.contains(QStringLiteral("property color accentColor")),
                 "InstrumentFrame must expose a typed accent color");
        QVERIFY2(frame.contains(QStringLiteral("Theme.clusterFrameInset")),
                 "InstrumentFrame geometry must come from Theme.qml");
        QVERIFY2(glass.contains(QStringLiteral("property color accentColor")),
                 "GlassPanel must expose the frame accent contract");
        QVERIFY2(glass.contains(QStringLiteral("InstrumentFrame")),
                 "GlassPanel must render the shared physical frame");
        QVERIFY2(music.contains(QStringLiteral("InstrumentFrame")),
                 "MusicPlayer must receive the shared physical frame");
        QVERIFY2(parking.contains(QStringLiteral("accentColor: root.proximityColor")),
                 "Parking must drive the frame accent from proximity state");
        QVERIFY2(center.contains(QStringLiteral("id: clusterStatusLeds")),
                 "CenterHub must expose the OEM status LED rail");
        QVERIFY2(center.contains(QStringLiteral("root.activePageAccent")),
                 "CenterHub tabs and LEDs must share the active page accent");
        QVERIFY2(center.contains(QStringLiteral("Theme.centerStatusLedWidth")),
                 "CenterHub LED geometry must come from Theme.qml");

        const QRegularExpression executableJs(
            QStringLiteral("\\b(function|if|for|while|switch|var|let|const)\\b"));
        QVERIFY2(!executableJs.match(frame).hasMatch(),
                 "InstrumentFrame.qml must contain no executable JavaScript keywords");
    }

    void testInitialValues() {
        VehicleStatusViewModel vm;
        QCOMPARE(vm.speed(), 0.0);
        QCOMPARE(vm.rpm(), 0);
        QCOMPARE(vm.gear(), QString("P"));
        QCOMPARE(vm.isWarning(), false);
        QCOMPARE(vm.displaySpeed(), 0);
    }

    void testSetSpeed() {
        VehicleStatusViewModel vm;
        QSignalSpy spy(&vm, &VehicleStatusViewModel::speedChanged);
        
        vm.setSpeed(120.5);
        QCOMPARE(vm.speed(), 120.5);
        QCOMPARE(vm.displaySpeed(), 121);
        QCOMPARE(spy.count(), 1);
        
        // Setting same value should not emit signal again
        vm.setSpeed(120.5);
        QCOMPARE(spy.count(), 1);
    }

    void testSetRpm() {
        VehicleStatusViewModel vm;
        QSignalSpy spy(&vm, &VehicleStatusViewModel::rpmChanged);
        
        vm.setRpm(3000);
        QCOMPARE(vm.rpm(), 3000);
        QCOMPARE(spy.count(), 1);
    }

    void testSetGear() {
        VehicleStatusViewModel vm;
        QSignalSpy spy(&vm, &VehicleStatusViewModel::gearChanged);
        
        vm.setGear("D");
        QCOMPARE(vm.gear(), QString("D"));
        QCOMPARE(spy.count(), 1);
    }

    void testSetWarning() {
        VehicleStatusViewModel vm;
        QSignalSpy spy(&vm, &VehicleStatusViewModel::isWarningChanged);
        
        vm.setIsWarning(true);
        QCOMPARE(vm.isWarning(), true);
        QCOMPARE(spy.count(), 1);
    }
    
    void testSetBattery() {
        VehicleStatusViewModel vm;
        QSignalSpy spy(&vm, &VehicleStatusViewModel::batteryChanged);

        vm.setBattery(72);
        QCOMPARE(vm.battery(), 72);
        QCOMPARE(spy.count(), 1);

        // Setting same value should not emit signal again
        vm.setBattery(72);
        QCOMPARE(spy.count(), 1);
    }

    void testSetRange() {
        VehicleStatusViewModel vm;
        QSignalSpy spy(&vm, &VehicleStatusViewModel::rangeChanged);

        vm.setRange(210);
        QCOMPARE(vm.range(), 210);
        QCOMPARE(spy.count(), 1);

        // Setting same value should not emit signal again
        vm.setRange(210);
        QCOMPARE(spy.count(), 1);
    }

    void testSetTemperature() {
        VehicleStatusViewModel vm;
        QSignalSpy spy(&vm, &VehicleStatusViewModel::temperatureChanged);

        vm.setTemperature(45);
        QCOMPARE(vm.temperature(), 45);
        QCOMPARE(spy.count(), 1);

        // Setting same value should not emit signal again
        vm.setTemperature(45);
        QCOMPARE(spy.count(), 1);
    }

    void testUpdateTelemetry() {
        VehicleStatusViewModel vm;
        
        vm.updateTelemetry(90.0, 3000, "5", false, 95, 300, 60);
        
        QCOMPARE(vm.speed(), 90.0);
        QCOMPARE(vm.displaySpeed(), 90);
        QCOMPARE(vm.rpm(), 3000);
        QCOMPARE(vm.gear(), QString("5"));
        QCOMPARE(vm.isWarning(), false);
        QCOMPARE(vm.battery(), 95);
        QCOMPARE(vm.range(), 300);
        QCOMPARE(vm.temperature(), 60);
        
        vm.updateTelemetry(90.6, 3000, "5", true);
        QCOMPARE(vm.displaySpeed(), 91);
        QCOMPARE(vm.isWarning(), true);
    }

    // --- ThemeViewModel (Phase 13) ---

    void testThemeDefaultIsNight() {
        ThemeViewModel theme;
        QCOMPARE(theme.isNight(), true);
    }

    void testToggleTheme() {
        ThemeViewModel theme;
        QSignalSpy spy(&theme, &ThemeViewModel::isNightChanged);

        theme.toggleTheme();
        QCOMPARE(theme.isNight(), false);
        QCOMPARE(spy.count(), 1);

        theme.toggleTheme();
        QCOMPARE(theme.isNight(), true);
        QCOMPARE(spy.count(), 2);
    }

    void testWindowMoveRequestedOnlyWhileDragIsActive() {
        ThemeViewModel theme;
        QSignalSpy spy(&theme, &ThemeViewModel::windowMoveRequested);

        theme.handleWindowDragActive(false);
        QCOMPARE(spy.size(), 0);

        theme.handleWindowDragActive(true);
        QCOMPARE(spy.size(), 1);
    }

    // --- TripComputerViewModel (Phase 15) ---

    void testTripDefaults() {
        TripComputerViewModel trip;
        QCOMPARE(trip.odometerKm(), 0.0);
        QCOMPARE(trip.tripKm(), 0.0);
        QCOMPARE(trip.avgSpeedKmh(), 0.0);
        QCOMPARE(trip.tripDisplay(), QString("0.0 km"));
        QCOMPARE(trip.odoDisplay(), QString("0 km"));
        QCOMPARE(trip.avgSpeedDisplay(), QString("0.0 km/h"));
    }

    void testTripAccumulatesDistance() {
        TripComputerViewModel trip(3600000); // maxDeltaMs wide open for math tests
        QSignalSpy spy(&trip, &TripComputerViewModel::tripChanged);

        trip.updateSpeed(60.0, 60000);   // 60 km/h for 1 min -> 1.0 km
        QCOMPARE(trip.tripKm(), 1.0);
        QCOMPARE(trip.odometerKm(), 1.0);
        QCOMPARE(spy.count(), 1);

        trip.updateSpeed(120.0, 30000);  // 120 km/h for 30 s -> +1.0 km
        QCOMPARE(trip.tripKm(), 2.0);
        QCOMPARE(trip.odometerKm(), 2.0);
        QCOMPARE(trip.tripDisplay(), QString("2.0 km"));
        QCOMPARE(trip.odoDisplay(), QString("2 km"));
        QCOMPARE(trip.avgSpeedDisplay(), QString("80.0 km/h"));
        QCOMPARE(spy.count(), 2);
    }

    void testTripAvgSpeedIncludesIdleTime() {
        TripComputerViewModel trip(3600000);
        trip.updateSpeed(60.0, 60000); // 1 km in 1 min
        trip.updateSpeed(0.0, 60000);  // idle 1 min still counts toward avg
        QCOMPARE(trip.tripKm(), 1.0);
        QCOMPARE(trip.avgSpeedKmh(), 30.0);
    }

    void testTripAvgSpeedGuardsZeroElapsed() {
        TripComputerViewModel trip;
        QSignalSpy spy(&trip, &TripComputerViewModel::tripChanged);
        QCOMPARE(trip.avgSpeedKmh(), 0.0); // no time elapsed -> no divide-by-zero

        trip.updateSpeed(50.0, 0);   // zero dt is a no-op
        trip.updateSpeed(50.0, -5);  // negative dt is a no-op
        QCOMPARE(trip.tripKm(), 0.0);
        QCOMPARE(trip.avgSpeedKmh(), 0.0);
        QCOMPARE(spy.count(), 0);
    }

    void testTripRejectsNonFiniteSpeed() {
        TripComputerViewModel trip;
        QSignalSpy spy(&trip, &TripComputerViewModel::tripChanged);

        trip.updateSpeed(std::numeric_limits<double>::infinity(), 1000);
        trip.updateSpeed(std::numeric_limits<double>::quiet_NaN(), 1000);

        QCOMPARE(trip.tripKm(), 0.0);
        QCOMPARE(trip.odometerKm(), 0.0);
        QCOMPARE(spy.count(), 0);
    }

    void testTripNormalizesNonPositiveMaxDelta() {
        TripComputerViewModel trip(0);
        trip.updateSpeed(36.0, 1000);

        QVERIFY(trip.tripKm() > 0.0);
        QVERIFY(trip.odometerKm() > 0.0);

        TripComputerViewModel negativeTrip(-1);
        negativeTrip.updateSpeed(36.0, 1000);
        QVERIFY(negativeTrip.tripKm() > 0.0);
        QVERIFY(negativeTrip.odometerKm() > 0.0);
    }

    void testTripSignalsOnlyWhenEffectiveStateChanges() {
        TripComputerViewModel trip;
        QSignalSpy spy(&trip, &TripComputerViewModel::tripChanged);

        trip.updateSpeed(0.0, 1000);
        QCOMPARE(spy.count(), 0);

        trip.updateSpeed(60.0, 60000);
        QCOMPARE(spy.count(), 1);

        trip.resetTrip();
        QCOMPARE(spy.count(), 2);
        trip.resetTrip();
        QCOMPARE(spy.count(), 2);
    }

    void testTripClampsStaleDelta() {
        TripComputerViewModel trip(100); // clamp deltas to 100 ms
        trip.updateSpeed(36.0, 5000);    // stale 5 s gap -> treated as 100 ms
        QVERIFY(qFuzzyCompare(trip.tripKm() + 1.0, 1.001)); // 36 km/h * 0.1 s = 1 m
    }

    void testResetTrip() {
        TripComputerViewModel trip(3600000);
        trip.updateSpeed(60.0, 60000);
        QSignalSpy spy(&trip, &TripComputerViewModel::tripChanged);

        trip.resetTrip();
        QCOMPARE(trip.tripKm(), 0.0);
        QCOMPARE(trip.avgSpeedKmh(), 0.0);
        QCOMPARE(trip.odometerKm(), 1.0); // odometer survives trip reset
        QCOMPARE(spy.count(), 1);

        trip.updateSpeed(60.0, 60000);    // trip restarts cleanly after reset
        QCOMPARE(trip.tripKm(), 1.0);
        QCOMPARE(trip.avgSpeedKmh(), 60.0);
        QCOMPARE(trip.odometerKm(), 2.0);
    }

};

QTEST_MAIN(TestViewModels)

#include "main.moc"
