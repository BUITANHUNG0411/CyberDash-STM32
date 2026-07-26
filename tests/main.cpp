#include <QtTest>
#include <QCoreApplication>
#include "viewmodels/VehicleStatusViewModel.h"
#include "viewmodels/ThemeViewModel.h"
#include "viewmodels/VehicleModeViewModel.h"

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
};

QTEST_MAIN(TestViewModels)

#include "main.moc"
