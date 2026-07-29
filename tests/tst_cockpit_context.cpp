#include <QSignalSpy>
#include <QtTest>

#include "services/MockSafetyScenarioService.h"
#include "viewmodels/CockpitContextViewModel.h"
#include "viewmodels/DriveModeViewModel.h"
#include "viewmodels/ParkingAssistViewModel.h"
#include "viewmodels/SafetyScenarioViewModel.h"
#include "viewmodels/ThemeViewModel.h"
#include "viewmodels/VehicleModeViewModel.h"

class TestCockpitContext final : public QObject
{
    Q_OBJECT

private slots:
    void exposesDefaultGlanceableLabels()
    {
        VehicleModeViewModel vehicleMode;
        DriveModeViewModel driveMode;
        ThemeViewModel theme;
        ParkingAssistViewModel parking;
        MockSafetyScenarioService safetyService;
        SafetyScenarioViewModel safetyScenario(&safetyService);
        safetyScenario.setPresentationAllowed(true);
        CockpitContextViewModel context(&vehicleMode, &driveMode, &theme, &parking, &safetyScenario);

        QCOMPARE(context.vehicleModeLabel(), QStringLiteral("CAR"));
        QCOMPARE(context.driveModeLabel(), QStringLiteral("NORMAL"));
        QCOMPARE(context.themeLabel(), QStringLiteral("NIGHT"));
        QCOMPARE(context.sourceLabel(), QStringLiteral("SIMULATOR"));
        QCOMPARE(context.safetyLabLabel(), QStringLiteral("LAB READY"));
        QVERIFY(context.safetyLabAvailable());
    }

    void followsVehicleDriveThemeAndSourceTransitions()
    {
        VehicleModeViewModel vehicleMode;
        DriveModeViewModel driveMode;
        ThemeViewModel theme;
        ParkingAssistViewModel parking;
        MockSafetyScenarioService safetyService;
        SafetyScenarioViewModel safetyScenario(&safetyService);
        safetyScenario.setPresentationAllowed(true);
        CockpitContextViewModel context(&vehicleMode, &driveMode, &theme, &parking, &safetyScenario);

        vehicleMode.cycleVehicleMode();
        QCOMPARE(context.vehicleModeLabel(), QStringLiteral("BIKE"));
        QCOMPARE(context.safetyLabLabel(), QStringLiteral("LAB: CAR ONLY"));
        QVERIFY(!context.safetyLabAvailable());

        vehicleMode.cycleVehicleMode();
        QCOMPARE(context.vehicleModeLabel(), QStringLiteral("SCOOTER"));

        vehicleMode.cycleVehicleMode();
        QCOMPARE(context.vehicleModeLabel(), QStringLiteral("CAR"));

        driveMode.cycleDriveMode();
        QCOMPARE(context.driveModeLabel(), QStringLiteral("SPORT"));
        theme.toggleTheme();
        QCOMPARE(context.themeLabel(), QStringLiteral("DAY"));

        context.setHardwareConnected(true);
        QCOMPARE(context.sourceLabel(), QStringLiteral("UART LIVE"));
        context.setHardwareConnected(false);
        QCOMPARE(context.sourceLabel(), QStringLiteral("SIMULATOR"));
    }

    void explainsSafetyLabPriorityStates()
    {
        VehicleModeViewModel vehicleMode;
        DriveModeViewModel driveMode;
        ThemeViewModel theme;
        ParkingAssistViewModel parking;
        MockSafetyScenarioService safetyService;
        SafetyScenarioViewModel safetyScenario(&safetyService);
        safetyScenario.setPresentationAllowed(true);
        CockpitContextViewModel context(&vehicleMode, &driveMode, &theme, &parking, &safetyScenario);

        safetyScenario.startDemo();
        QCOMPARE(context.safetyLabLabel(), QStringLiteral("LAB ACTIVE"));
        QVERIFY(!context.safetyLabAvailable());

        safetyScenario.stopDemo();
        parking.updateSensorSample(29, true);
        QCOMPARE(context.safetyLabLabel(), QStringLiteral("LAB: PARKING"));
        QVERIFY(!context.safetyLabAvailable());

        parking.updateSensorSample(90, true);
        QCOMPARE(context.safetyLabLabel(), QStringLiteral("LAB READY"));
        QVERIFY(context.safetyLabAvailable());
    }

    void emitsOnlyWhenEffectiveContextChanges()
    {
        VehicleModeViewModel vehicleMode;
        DriveModeViewModel driveMode;
        ThemeViewModel theme;
        ParkingAssistViewModel parking;
        MockSafetyScenarioService safetyService;
        SafetyScenarioViewModel safetyScenario(&safetyService);
        safetyScenario.setPresentationAllowed(true);
        CockpitContextViewModel context(&vehicleMode, &driveMode, &theme, &parking, &safetyScenario);
        QSignalSpy sourceSpy(&context, &CockpitContextViewModel::sourceLabelChanged);
        QSignalSpy safetyLabelSpy(&context, &CockpitContextViewModel::safetyLabLabelChanged);
        QSignalSpy safetyAvailabilitySpy(&context, &CockpitContextViewModel::safetyLabAvailableChanged);

        context.setHardwareConnected(false);
        QCOMPARE(sourceSpy.count(), 0);
        context.setHardwareConnected(true);
        QCOMPARE(sourceSpy.count(), 1);
        context.setHardwareConnected(true);
        QCOMPARE(sourceSpy.count(), 1);

        parking.updateSensorSample(90, true);
        QCOMPARE(safetyLabelSpy.count(), 0);
        QCOMPARE(safetyAvailabilitySpy.count(), 0);
        parking.updateSensorSample(29, true);
        QCOMPARE(safetyLabelSpy.count(), 1);
        QCOMPARE(safetyAvailabilitySpy.count(), 1);
        parking.updateSensorSample(29, true);
        QCOMPARE(safetyLabelSpy.count(), 1);
        QCOMPARE(safetyAvailabilitySpy.count(), 1);
    }
};

QTEST_MAIN(TestCockpitContext)
#include "tst_cockpit_context.moc"
