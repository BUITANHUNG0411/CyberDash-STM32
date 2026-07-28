#include <QtTest>
#include <QSignalSpy>
#include "services/MockParkingSensorService.h"
#include "viewmodels/CenterHubViewModel.h"
#include "viewmodels/ParkingAssistViewModel.h"

class TestParkingAssist final : public QObject
{
    Q_OBJECT

private slots:
    void classifiesDistanceThresholds()
    {
        ParkingAssistViewModel viewModel;
        viewModel.updateSensorSample(250, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Clear);
        QCOMPARE(viewModel.statusText(), QStringLiteral("REAR CLEAR"));
        QCOMPARE(viewModel.distanceText(), QStringLiteral("250 CM"));

        viewModel.updateSensorSample(150, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        viewModel.updateSensorSample(31, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        viewModel.updateSensorSample(30, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
        QCOMPARE(viewModel.statusText(), QStringLiteral("STOP"));
        viewModel.updateSensorSample(1, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
    }

    void invalidInputBecomesUnavailable()
    {
        ParkingAssistViewModel viewModel;
        viewModel.updateSensorSample(80, true);
        viewModel.updateSensorSample(0, true);
        QVERIFY(!viewModel.sensorAvailable());
        QCOMPARE(viewModel.rearDistanceCm(), 0);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Unavailable);
        QCOMPARE(viewModel.distanceText(), QStringLiteral("—"));
        QCOMPARE(viewModel.statusText(), QStringLiteral("SENSOR UNAVAILABLE"));
        viewModel.updateSensorSample(-1, true);
        QVERIFY(!viewModel.sensorAvailable());
        viewModel.updateSensorSample(251, true);
        QVERIFY(!viewModel.sensorAvailable());
    }

    void reverseStateSelectsParkingPage()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
        parking.updateSensorSample(90, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        parking.updateSensorSample(0, false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
    }

    void identicalSampleDoesNotEmitDistanceChanged()
    {
        ParkingAssistViewModel viewModel;
        QSignalSpy spy(&viewModel, &ParkingAssistViewModel::rearDistanceChanged);
        viewModel.updateSensorSample(90, true);
        QCOMPARE(spy.count(), 1);
        viewModel.updateSensorSample(90, true);
        QCOMPARE(spy.count(), 1);
    }

    void validReverseSampleExpiresAfterStaleInterval()
    {
        ParkingAssistViewModel viewModel(1000);
        viewModel.updateSensorSample(90, true);
        viewModel.advanceStaleClock(999);
        QVERIFY(viewModel.sensorAvailable());
        viewModel.advanceStaleClock(1);
        QVERIFY(!viewModel.sensorAvailable());
        QVERIFY(viewModel.reverseActive());
    }

    void mockSequenceCoversAllPresentationStates()
    {
        MockParkingSensorService service;
        ParkingAssistViewModel viewModel;
        QObject::connect(&service, &MockParkingSensorService::parkingSampleUpdated,
                         &viewModel, &ParkingAssistViewModel::updateSensorSample);
        service.start();
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Unavailable);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Clear);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Unavailable);
    }
};

QTEST_MAIN(TestParkingAssist)
#include "tst_parking_assist.moc"
