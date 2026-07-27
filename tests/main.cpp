#include <QtTest>
#include <QCoreApplication>
#include <cmath>
#include <limits>
#include "viewmodels/VehicleStatusViewModel.h"
#include "viewmodels/ThemeViewModel.h"
#include "viewmodels/VehicleModeViewModel.h"
#include "viewmodels/DriveModeViewModel.h"
#include "viewmodels/TripComputerViewModel.h"
#include "viewmodels/MapViewModel.h"

class TestViewModels : public QObject
{
    Q_OBJECT

public:
    TestViewModels() {}
    ~TestViewModels() {}

private slots:
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
        QCOMPARE(theme.bootStage(), 0);
        QCOMPARE(theme.isBooting(), true);
        QCOMPARE(theme.bootProgress(), 0.0);
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

    void testBootSequence() {
        ThemeViewModel theme(10); // 10 ms per sweep leg -> full boot ~20 ms
        QSignalSpy stageSpy(&theme, &ThemeViewModel::bootStageChanged);

        qreal maxProgress = 0.0;
        connect(&theme, &ThemeViewModel::bootProgressChanged, this, [&]() {
            maxProgress = qMax(maxProgress, theme.bootProgress());
        });

        theme.startBootSequence();
        QCOMPARE(theme.bootStage(), 1);

        QTRY_COMPARE_WITH_TIMEOUT(theme.bootStage(), 2, 2000);
        QCOMPARE(theme.isBooting(), false);
        QVERIFY(maxProgress > 0.9);
        QCOMPARE(theme.bootProgress(), 0.0);
        QCOMPARE(stageSpy.count(), 2); // 0->1 and 1->2
    }

    void testBootSequenceIsIdempotent() {
        ThemeViewModel theme(10);
        QSignalSpy stageSpy(&theme, &ThemeViewModel::bootStageChanged);

        theme.startBootSequence();
        theme.startBootSequence(); // must be a no-op

        QTRY_COMPARE_WITH_TIMEOUT(theme.bootStage(), 2, 2000);
        QCOMPARE(stageSpy.count(), 2);
    }

    void testToggleDuringBoot() {
        ThemeViewModel theme(50);
        theme.startBootSequence();
        QCOMPARE(theme.bootStage(), 1);

        theme.toggleTheme(); // theme and boot are independent
        QCOMPARE(theme.isNight(), false);

        QTRY_COMPARE_WITH_TIMEOUT(theme.bootStage(), 2, 2000);
        QCOMPARE(theme.isNight(), false);
    }

    // --- VehicleModeViewModel (Phase 14) ---

    void testVehicleModeDefaultIsCar() {
        VehicleModeViewModel mode;
        QCOMPARE(mode.vehicleMode(), QString("car"));
    }

    void testCycleVehicleMode() {
        VehicleModeViewModel mode;
        QSignalSpy spy(&mode, &VehicleModeViewModel::vehicleModeChanged);

        mode.cycleVehicleMode();
        QCOMPARE(mode.vehicleMode(), QString("bike"));
        QCOMPARE(spy.count(), 1);

        mode.cycleVehicleMode();
        QCOMPARE(mode.vehicleMode(), QString("scooter"));
        QCOMPARE(spy.count(), 2);

        mode.cycleVehicleMode();
        QCOMPARE(mode.vehicleMode(), QString("car"));
        QCOMPARE(spy.count(), 3);
    }

    void testCycleVehicleModeWrapsRepeatedly() {
        VehicleModeViewModel mode;
        QSignalSpy spy(&mode, &VehicleModeViewModel::vehicleModeChanged);

        mode.cycleVehicleMode(); mode.cycleVehicleMode(); mode.cycleVehicleMode();
        mode.cycleVehicleMode(); mode.cycleVehicleMode(); mode.cycleVehicleMode();
        QCOMPARE(mode.vehicleMode(), QString("car"));
        QCOMPARE(spy.count(), 6);
    }

    // --- DriveModeViewModel (Phase 15) ---

    void testDriveModeDefaultIsNormal() {
        DriveModeViewModel mode;
        QCOMPARE(mode.driveMode(), QString("normal"));
        QCOMPARE(mode.driveModeLabel(), QString("NORMAL"));
    }

    void testCycleDriveMode() {
        DriveModeViewModel mode;
        QSignalSpy spy(&mode, &DriveModeViewModel::driveModeChanged);

        mode.cycleDriveMode();
        QCOMPARE(mode.driveMode(), QString("sport"));
        QCOMPARE(mode.driveModeLabel(), QString("SPORT"));
        QCOMPARE(spy.count(), 1);

        mode.cycleDriveMode();
        QCOMPARE(mode.driveMode(), QString("eco"));
        QCOMPARE(mode.driveModeLabel(), QString("ECO"));
        QCOMPARE(spy.count(), 2);

        mode.cycleDriveMode();
        QCOMPARE(mode.driveMode(), QString("normal"));
        QCOMPARE(spy.count(), 3);
    }

    void testCycleDriveModeWrapsRepeatedly() {
        DriveModeViewModel mode;
        QSignalSpy spy(&mode, &DriveModeViewModel::driveModeChanged);

        mode.cycleDriveMode(); mode.cycleDriveMode(); mode.cycleDriveMode();
        mode.cycleDriveMode(); mode.cycleDriveMode(); mode.cycleDriveMode();
        QCOMPARE(mode.driveMode(), QString("normal"));
        QCOMPARE(spy.count(), 6);
    }

    // --- TripComputerViewModel (Phase 15) ---

    void testTripDefaults() {
        TripComputerViewModel trip;
        QCOMPARE(trip.odometerKm(), 0.0);
        QCOMPARE(trip.tripKm(), 0.0);
        QCOMPARE(trip.avgSpeedKmh(), 0.0);
        QCOMPARE(trip.tripDisplay(), QString("0.0 km"));
        QCOMPARE(trip.odoDisplay(), QString("0 km"));
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

    // --- MapViewModel (Phase 16) ---

    void testMapDefaults() {
        MapViewModel map;
        QCOMPARE(map.routeProgress(), 0.0);
    }

    void testMapProgressAdvances() {
        MapViewModel map(2.0);
        QSignalSpy spy(&map, &MapViewModel::routeProgressChanged);

        map.updateDistance(0.5); // 0.5 km on a 2.0 km loop -> 0.25
        QCOMPARE(map.routeProgress(), 0.25);
        QCOMPARE(spy.count(), 1);

        map.updateDistance(0.5); // same value -> no re-emit
        QCOMPARE(spy.count(), 1);
    }

    void testMapProgressWraps() {
        MapViewModel map(2.0);
        map.updateDistance(2.5); // wraps past 1.0 -> 0.25
        QCOMPARE(map.routeProgress(), 0.25);
    }

    void testMapInvalidRouteLengthsUseSafeDefault() {
        const double invalidLengths[] = {
            0.0,
            -2.0,
            (std::numeric_limits<double>::infinity)(),
            std::numeric_limits<double>::quiet_NaN()
        };

        for (double routeLength : invalidLengths) {
            MapViewModel map(routeLength);
            map.updateDistance(0.5);
            QVERIFY(std::isfinite(map.routeProgress()));
            QCOMPARE(map.routeProgress(), 0.25);
        }
    }

    void testMapInvalidDistancesResetToRouteStart() {
        MapViewModel map(2.0);
        map.updateDistance(0.5);
        QCOMPARE(map.routeProgress(), 0.25);

        const double invalidDistances[] = {
            -1.0,
            (std::numeric_limits<double>::infinity)(),
            std::numeric_limits<double>::quiet_NaN()
        };

        for (double distance : invalidDistances) {
            map.updateDistance(distance);
            QVERIFY(std::isfinite(map.routeProgress()));
            QCOMPARE(map.routeProgress(), 0.0);
        }
    }
};

QTEST_MAIN(TestViewModels)

#include "main.moc"
