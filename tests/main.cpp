#include <QtTest>
#include <QCoreApplication>
#include "viewmodels/VehicleStatusViewModel.h"

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
};

QTEST_MAIN(TestViewModels)

#include "main.moc"
