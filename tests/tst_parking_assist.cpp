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
        QCOMPARE(viewModel.statusText(), QStringLiteral("REAR CAUTION"));
        viewModel.updateSensorSample(31, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        viewModel.updateSensorSample(30, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
        QCOMPARE(viewModel.statusText(), QStringLiteral("STOP"));
        viewModel.updateSensorSample(1, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
    }

    void hysteresisPreventsThresholdFlicker()
    {
        ParkingAssistViewModel viewModel;
        viewModel.updateSensorSample(250, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Clear);

        viewModel.updateSensorSample(150, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        viewModel.updateSensorSample(152, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        viewModel.updateSensorSample(156, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Clear);

        viewModel.updateSensorSample(30, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
        viewModel.updateSensorSample(34, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
        viewModel.updateSensorSample(35, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
    }

    void healthStateDistinguishesLiveStaleAndUnavailable()
    {
        ParkingAssistViewModel viewModel(1000);
        QSignalSpy healthSpy(&viewModel, &ParkingAssistViewModel::sensorHealthChanged);
        viewModel.updateSensorSample(90, true);
        QCOMPARE(viewModel.sensorHealth(), ParkingAssistViewModel::Live);
        QCOMPARE(viewModel.healthText(), QStringLiteral("ULTRASONIC LIVE"));

        viewModel.advanceStaleClock(1000);
        QCOMPARE(viewModel.sensorHealth(), ParkingAssistViewModel::Stale);
        QVERIFY(!viewModel.sensorAvailable());
        QCOMPARE(viewModel.healthText(), QStringLiteral("ULTRASONIC STALE"));
        QCOMPARE(viewModel.statusText(), QStringLiteral("SENSOR STALE"));

        viewModel.updateSensorSample(90, true);
        QCOMPARE(viewModel.sensorHealth(), ParkingAssistViewModel::Live);
        viewModel.updateSensorSample(0, true);
        QCOMPARE(viewModel.sensorHealth(), ParkingAssistViewModel::SensorUnavailable);
        QCOMPARE(viewModel.healthText(), QStringLiteral("ULTRASONIC UNAVAILABLE"));
        QCOMPARE(healthSpy.count(), 4);
    }

    void derivesPresentationSafeProximityProgress()
    {
        ParkingAssistViewModel viewModel;
        QSignalSpy segmentsSpy(&viewModel, &ParkingAssistViewModel::proximitySegmentsChanged);
        QSignalSpy progressSpy(&viewModel, &ParkingAssistViewModel::proximityProgressChanged);

        viewModel.updateSensorSample(250, true);
        QCOMPARE(viewModel.proximitySegments(), 1);
        QCOMPARE(viewModel.proximityProgress(), 0.0);

        viewModel.updateSensorSample(150, true);
        QCOMPARE(viewModel.proximitySegments(), 4);
        QVERIFY(qAbs(viewModel.proximityProgress() - (100.0 / 220.0)) < 0.000001);

        viewModel.updateSensorSample(30, true);
        QCOMPARE(viewModel.proximitySegments(), 8);
        QCOMPARE(viewModel.proximityProgress(), 1.0);

        viewModel.updateSensorSample(0, true);
        QCOMPARE(viewModel.proximitySegments(), 0);
        QCOMPARE(viewModel.proximityProgress(), 0.0);

        QCOMPARE(segmentsSpy.count(), 4);
        QCOMPARE(progressSpy.count(), 3);
        viewModel.updateSensorSample(0, true);
        QCOMPARE(segmentsSpy.count(), 4);
        QCOMPARE(progressSpy.count(), 3);
    }

    void formatsAnimatedDistanceInCPlusPlus()
    {
        ParkingAssistViewModel viewModel;
        QCOMPARE(viewModel.formatDistance(249.4), QStringLiteral("249 CM"));
        QCOMPARE(viewModel.formatDistance(249.6), QStringLiteral("250 CM"));
        QCOMPARE(viewModel.formatDistance(0.0), QStringLiteral("—"));
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

    void centerHubShowsParkingOnlyBelowCriticalDistance()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
        parking.updateSensorSample(90, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
        parking.updateSensorSample(30, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
        parking.updateSensorSample(29, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        parking.updateSensorSample(35, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
        parking.updateSensorSample(29, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        parking.updateSensorSample(0, false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
    }

    void centerHubSupportsManualHorizontalSwipe()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);

        hub.setSwipeActive(true);
        hub.updateSwipeTranslation(-79.0);
        hub.setSwipeActive(false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);

        hub.setSwipeActive(true);
        hub.updateSwipeTranslation(-80.0);
        hub.setSwipeActive(false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);

        hub.setSwipeActive(true);
        hub.updateSwipeTranslation(-80.0);
        hub.setSwipeActive(false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);

        hub.setSwipeActive(true);
        hub.updateSwipeTranslation(80.0);
        hub.setSwipeActive(false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);

        hub.setSwipeActive(true);
        hub.updateSwipeTranslation(80.0);
        hub.setSwipeActive(false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
    }

    void centerHubRejectsInvalidPageRequest()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);

        QVERIFY(!hub.selectPage(2));
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
    }

    void centerHubManualSelectionHonorsCriticalSafetyOverride()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);

        QVERIFY(hub.selectPage(CenterHubViewModel::ParkingPage));
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        QVERIFY(hub.selectPage(CenterHubViewModel::MusicPage));
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);

        parking.updateSensorSample(29, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        QVERIFY(!hub.selectPage(CenterHubViewModel::MusicPage));
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        QVERIFY(!hub.selectPage(2));
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        QVERIFY(!hub.selectPage(42));
    }

    void nullParkingViewModelDefaultsToMusicPage()
    {
        CenterHubViewModel hub(nullptr);
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
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Unavailable);

        // STOP remains available through the high-level sample seam for a manual/mock override.
        viewModel.updateSensorSample(29, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
    }
};

QTEST_MAIN(TestParkingAssist)
#include "tst_parking_assist.moc"
