#include <QSignalSpy>
#include <QtTest>

#include "services/MockSafetyScenarioService.h"
#include "viewmodels/SafetyScenarioViewModel.h"

namespace {
constexpr qreal kRiskTolerance = 0.001;

bool isNear(qreal actual, qreal expected)
{
    return qAbs(actual - expected) <= kRiskTolerance;
}
} // namespace

class TestSafetyScenario final : public QObject
{
    Q_OBJECT

private slots:
    void startsInNormalFrame()
    {
        MockSafetyScenarioService service;

        service.start();

        QVERIFY(service.isRunning());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Normal);
        QCOMPARE(service.severity(), MockSafetyScenarioService::Severity::None);
        QCOMPARE(service.riskProgress(), 0.0);
        QCOMPARE(service.riskSegments(), 0);
        QCOMPARE(service.threatPosition(), 0.0);
        QVERIFY(!service.pulseActive());
        QVERIFY(!service.acknowledgementAvailable());
    }

    void phaseBoundaries_data()
    {
        QTest::addColumn<qint64>("elapsedMs");
        QTest::addColumn<MockSafetyScenarioService::Phase>("expectedPhase");
        QTest::addColumn<MockSafetyScenarioService::Severity>("expectedSeverity");

        QTest::newRow("normal-start") << qint64{0}
                                       << MockSafetyScenarioService::Phase::Normal
                                       << MockSafetyScenarioService::Severity::None;
        QTest::newRow("normal-end") << qint64{5999}
                                     << MockSafetyScenarioService::Phase::Normal
                                     << MockSafetyScenarioService::Severity::None;
        QTest::newRow("advisory-start") << qint64{6000}
                                         << MockSafetyScenarioService::Phase::Advisory
                                         << MockSafetyScenarioService::Severity::Advisory;
        QTest::newRow("advisory-end") << qint64{19999}
                                       << MockSafetyScenarioService::Phase::Advisory
                                       << MockSafetyScenarioService::Severity::Advisory;
        QTest::newRow("critical-start") << qint64{20000}
                                         << MockSafetyScenarioService::Phase::Critical
                                         << MockSafetyScenarioService::Severity::Critical;
        QTest::newRow("critical-end") << qint64{47999}
                                       << MockSafetyScenarioService::Phase::Critical
                                       << MockSafetyScenarioService::Severity::Critical;
        QTest::newRow("recovery-start") << qint64{48000}
                                         << MockSafetyScenarioService::Phase::Recovery
                                         << MockSafetyScenarioService::Severity::Advisory;
        QTest::newRow("recovery-end") << qint64{71999}
                                       << MockSafetyScenarioService::Phase::Recovery
                                       << MockSafetyScenarioService::Severity::Advisory;
        QTest::newRow("complete") << qint64{72000}
                                   << MockSafetyScenarioService::Phase::Complete
                                   << MockSafetyScenarioService::Severity::None;
    }

    void phaseBoundaries()
    {
        QFETCH(qint64, elapsedMs);
        QFETCH(MockSafetyScenarioService::Phase, expectedPhase);
        QFETCH(MockSafetyScenarioService::Severity, expectedSeverity);
        MockSafetyScenarioService service;

        service.start();
        service.advance(elapsedMs);

        QCOMPARE(service.phase(), expectedPhase);
        QCOMPARE(service.severity(), expectedSeverity);
        QVERIFY(service.riskProgress() >= 0.0);
        QVERIFY(service.riskProgress() <= 1.0);
        QVERIFY(service.riskSegments() >= 0);
        QVERIFY(service.riskSegments() <= 8);
        QVERIFY(service.threatPosition() >= 0.0);
        QVERIFY(service.threatPosition() <= 1.0);
    }

    void visualTimelineContract_data()
    {
        QTest::addColumn<qint64>("elapsedMs");
        QTest::addColumn<MockSafetyScenarioService::Phase>("expectedPhase");
        QTest::addColumn<MockSafetyScenarioService::Severity>("expectedSeverity");
        QTest::addColumn<qreal>("expectedRisk");
        QTest::addColumn<bool>("mustBeBelowExpectedRisk");
        QTest::addColumn<bool>("mustBeAboveExpectedRisk");
        QTest::addColumn<bool>("expectedPulse");

        QTest::newRow("advisory-start") << qint64{6000}
                                          << MockSafetyScenarioService::Phase::Advisory
                                          << MockSafetyScenarioService::Severity::Advisory
                                          << qreal{0.20} << false << false << false;
        QTest::newRow("advisory-before-critical") << qint64{19999}
                                                    << MockSafetyScenarioService::Phase::Advisory
                                                    << MockSafetyScenarioService::Severity::Advisory
                                                    << qreal{0.70} << true << false << false;
        QTest::newRow("critical") << qint64{20000}
                                    << MockSafetyScenarioService::Phase::Critical
                                    << MockSafetyScenarioService::Severity::Critical
                                    << qreal{1.0} << false << false << true;
        QTest::newRow("recovery-start") << qint64{48000}
                                          << MockSafetyScenarioService::Phase::Recovery
                                          << MockSafetyScenarioService::Severity::Advisory
                                          << qreal{1.0} << false << false << false;
        QTest::newRow("recovery-before-complete") << qint64{71999}
                                                    << MockSafetyScenarioService::Phase::Recovery
                                                    << MockSafetyScenarioService::Severity::Advisory
                                                    << qreal{0.0} << false << true << false;
        QTest::newRow("complete") << qint64{72000}
                                    << MockSafetyScenarioService::Phase::Complete
                                    << MockSafetyScenarioService::Severity::None
                                    << qreal{0.0} << false << false << false;
    }

    void visualTimelineContract()
    {
        QFETCH(qint64, elapsedMs);
        QFETCH(MockSafetyScenarioService::Phase, expectedPhase);
        QFETCH(MockSafetyScenarioService::Severity, expectedSeverity);
        QFETCH(qreal, expectedRisk);
        QFETCH(bool, mustBeBelowExpectedRisk);
        QFETCH(bool, mustBeAboveExpectedRisk);
        QFETCH(bool, expectedPulse);
        MockSafetyScenarioService service;

        service.start();
        service.advance(elapsedMs);

        QCOMPARE(service.phase(), expectedPhase);
        QCOMPARE(service.severity(), expectedSeverity);
        QVERIFY2(isNear(service.riskProgress(), expectedRisk), "risk must be near its timeline value");
        if (mustBeBelowExpectedRisk) {
            QVERIFY(service.riskProgress() < expectedRisk);
        }
        if (mustBeAboveExpectedRisk) {
            QVERIFY(service.riskProgress() > expectedRisk);
        }
        QCOMPARE(service.pulseActive(), expectedPulse);
    }

    void threatPositionRemainsBoundedWithEndpoints_data()
    {
        QTest::addColumn<qint64>("elapsedMs");
        QTest::addColumn<qreal>("expectedPosition");

        QTest::newRow("start") << qint64{0} << qreal{0.0};
        QTest::newRow("midpoint") << qint64{36000} << qreal{0.5};
        QTest::newRow("complete") << qint64{72000} << qreal{1.0};
    }

    void threatPositionRemainsBoundedWithEndpoints()
    {
        QFETCH(qint64, elapsedMs);
        QFETCH(qreal, expectedPosition);
        MockSafetyScenarioService service;

        service.start();
        service.advance(elapsedMs);

        QVERIFY(service.threatPosition() >= 0.0);
        QVERIFY(service.threatPosition() <= 1.0);
        QVERIFY2(isNear(service.threatPosition(), expectedPosition), "threat position must reach its timeline endpoint");
    }

    void acknowledgementOnlyWorksInAcknowledgementWindow()
    {
        MockSafetyScenarioService service;

        service.start();
        service.advance(33999);
        QVERIFY(!service.acknowledgementAvailable());
        QVERIFY(!service.acknowledge());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Critical);

        service.replay();
        service.advance(34000);
        QVERIFY(service.acknowledgementAvailable());
        QVERIFY(service.acknowledge());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Recovery);
        QCOMPARE(service.severity(), MockSafetyScenarioService::Severity::Advisory);
        QVERIFY(!service.acknowledgementAvailable());

        service.replay();
        service.advance(47999);
        QVERIFY(service.acknowledgementAvailable());
        QVERIFY(service.acknowledge());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Recovery);

        service.replay();
        service.advance(48000);
        QVERIFY(!service.acknowledgementAvailable());
        QVERIFY(!service.acknowledge());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Recovery);
    }

    void replayResetsTheElapsedTimeline()
    {
        MockSafetyScenarioService service;

        service.start();
        service.advance(20000);
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Critical);
        service.replay();

        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Normal);
        QCOMPARE(service.riskProgress(), 0.0);
        QCOMPARE(service.riskSegments(), 0);
    }

    void stopReturnsIdleFrame()
    {
        MockSafetyScenarioService service;

        service.start();
        service.advance(20000);
        service.stop();

        QVERIFY(!service.isRunning());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Idle);
        QCOMPARE(service.severity(), MockSafetyScenarioService::Severity::None);
        QVERIFY(isNear(service.riskProgress(), 0.0));
        QCOMPARE(service.riskSegments(), 0);
        QVERIFY(isNear(service.threatPosition(), 0.0));
        QVERIFY(!service.pulseActive());
        QVERIFY(!service.acknowledgementAvailable());
    }

    void naturalCompletionStopsAndPreservesTerminalFrame()
    {
        MockSafetyScenarioService service;
        QSignalSpy frameSpy(&service, &MockSafetyScenarioService::frameChanged);
        QSignalSpy runningSpy(&service, &MockSafetyScenarioService::runningChanged);

        service.start();
        service.advance(72000);

        QVERIFY(!service.isRunning());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Complete);
        QCOMPARE(service.severity(), MockSafetyScenarioService::Severity::None);
        QVERIFY(isNear(service.riskProgress(), 0.0));
        QCOMPARE(service.riskSegments(), 0);
        QVERIFY(isNear(service.threatPosition(), 1.0));
        QVERIFY(!service.pulseActive());
        QVERIFY(!service.acknowledgementAvailable());
        QCOMPARE(frameSpy.size(), 2);
        QCOMPARE(runningSpy.size(), 2);

        service.advance(MockSafetyScenarioService::updateIntervalMs());
        QCOMPARE(frameSpy.size(), 2);
        QCOMPARE(runningSpy.size(), 2);

        service.replay();
        QVERIFY(service.isRunning());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Normal);
        QCOMPARE(service.severity(), MockSafetyScenarioService::Severity::None);
        QVERIFY(isNear(service.riskProgress(), 0.0));
        QCOMPARE(service.riskSegments(), 0);
        QCOMPARE(frameSpy.size(), 3);
        QCOMPARE(runningSpy.size(), 3);
    }

    void nonPositiveAdvanceDoesNotChangeTheFrame()
    {
        MockSafetyScenarioService service;
        service.start();
        service.advance(6000);
        const auto phase = service.phase();
        const auto severity = service.severity();
        const auto riskProgress = service.riskProgress();
        const auto riskSegments = service.riskSegments();
        const auto threatPosition = service.threatPosition();
        const auto pulseActive = service.pulseActive();
        const auto acknowledgementAvailable = service.acknowledgementAvailable();

        service.advance(0);
        service.advance(-100);

        QCOMPARE(service.phase(), phase);
        QCOMPARE(service.severity(), severity);
        QCOMPARE(service.riskProgress(), riskProgress);
        QCOMPARE(service.riskSegments(), riskSegments);
        QCOMPARE(service.threatPosition(), threatPosition);
        QCOMPARE(service.pulseActive(), pulseActive);
        QCOMPARE(service.acknowledgementAvailable(), acknowledgementAvailable);
    }

    void frameNotificationsAreSuppressedForUnchangedState()
    {
        MockSafetyScenarioService service;
        QSignalSpy frameSpy(&service, &MockSafetyScenarioService::frameChanged);
        QSignalSpy runningSpy(&service, &MockSafetyScenarioService::runningChanged);

        service.start();
        QCOMPARE(frameSpy.size(), 1);
        QCOMPARE(runningSpy.size(), 1);

        service.start();
        QCOMPARE(frameSpy.size(), 1);
        QCOMPARE(runningSpy.size(), 1);

        service.advance(72000);
        QCOMPARE(frameSpy.size(), 2);
        QCOMPARE(runningSpy.size(), 2);
        service.advance(MockSafetyScenarioService::updateIntervalMs());
        QCOMPARE(frameSpy.size(), 2);
        QCOMPARE(runningSpy.size(), 2);
    }

    void viewModelMapsScriptToPresentationText()
    {
        MockSafetyScenarioService service;
        SafetyScenarioViewModel viewModel(&service);

        QVERIFY(!viewModel.presentationVisible());
        QVERIFY(!viewModel.canStart());
        QCOMPARE(viewModel.disclaimerText(), QStringLiteral("DEMO ONLY — NO REAL SENSOR / NO VEHICLE CONTROL"));

        viewModel.setPresentationAllowed(true);
        QVERIFY(viewModel.canStart());
        viewModel.startDemo();

        QVERIFY(viewModel.isActive());
        QVERIFY(viewModel.presentationVisible());
        QVERIFY(!viewModel.canStart());
        QCOMPARE(viewModel.title(), QStringLiteral("SAFETY LAB"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("FORWARD HAZARD SIMULATION"));

        service.advance(6000);
        QCOMPARE(viewModel.title(), QStringLiteral("FORWARD HAZARD"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("CAUTION: CLOSING DISTANCE"));
        QVERIFY(viewModel.advisoryActive());
        QVERIFY(!viewModel.criticalActive());

        service.advance(14000);
        QCOMPARE(viewModel.title(), QStringLiteral("BRAKE ADVISORY"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("ACKNOWLEDGE TO BEGIN RECOVERY"));
        QVERIFY(!viewModel.advisoryActive());
        QVERIFY(viewModel.criticalActive());
        QVERIFY(viewModel.pulseActive());

        service.advance(28000);
        QCOMPARE(viewModel.title(), QStringLiteral("RISK CLEARING"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("HAZARD WINDOW REDUCING"));
        QVERIFY(viewModel.advisoryActive());
        QVERIFY(!viewModel.criticalActive());
        QVERIFY(!viewModel.pulseActive());
    }

    void acknowledgementTransitionsToRecoveryThroughTheViewModel()
    {
        MockSafetyScenarioService service;
        SafetyScenarioViewModel viewModel(&service);
        viewModel.setPresentationAllowed(true);
        viewModel.startDemo();
        service.advance(34000);

        QVERIFY(viewModel.criticalActive());
        QVERIFY(viewModel.acknowledgementAvailable());

        viewModel.acknowledge();

        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Recovery);
        QCOMPARE(viewModel.title(), QStringLiteral("RISK CLEARING"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("HAZARD WINDOW REDUCING"));
        QVERIFY(viewModel.advisoryActive());
        QVERIFY(!viewModel.criticalActive());
        QVERIFY(!viewModel.acknowledgementAvailable());
    }

    void availabilityGateStopsAndHidesTheLab()
    {
        MockSafetyScenarioService service;
        SafetyScenarioViewModel viewModel(&service);
        viewModel.setPresentationAllowed(true);
        viewModel.startDemo();

        QVERIFY(viewModel.isActive());
        QVERIFY(viewModel.presentationVisible());
        QVERIFY(!viewModel.canStart());

        viewModel.setPresentationAllowed(false);

        QVERIFY(!service.isRunning());
        QVERIFY(!viewModel.isActive());
        QVERIFY(!viewModel.presentationVisible());
        QVERIFY(!viewModel.canStart());
    }

    void serviceDestructionSynchronizesViewModelToSafeInactiveState()
    {
        auto *service = new MockSafetyScenarioService;
        SafetyScenarioViewModel viewModel(service);
        viewModel.setPresentationAllowed(true);
        viewModel.startDemo();
        service->advance(34000);

        QVERIFY(viewModel.isActive());
        QVERIFY(viewModel.presentationVisible());
        QVERIFY(!viewModel.canStart());
        QVERIFY(viewModel.criticalActive());
        QVERIFY(viewModel.pulseActive());
        QVERIFY(viewModel.acknowledgementAvailable());

        QSignalSpy activeSpy(&viewModel, &SafetyScenarioViewModel::activeChanged);
        QSignalSpy presentationVisibleSpy(&viewModel, &SafetyScenarioViewModel::presentationVisibleChanged);
        QSignalSpy canStartSpy(&viewModel, &SafetyScenarioViewModel::canStartChanged);
        QSignalSpy displaySpy(&viewModel, &SafetyScenarioViewModel::displayChanged);
        QSignalSpy riskProgressSpy(&viewModel, &SafetyScenarioViewModel::riskProgressChanged);
        QSignalSpy riskSegmentsSpy(&viewModel, &SafetyScenarioViewModel::riskSegmentsChanged);
        QSignalSpy threatPositionSpy(&viewModel, &SafetyScenarioViewModel::threatPositionChanged);
        QSignalSpy criticalActiveSpy(&viewModel, &SafetyScenarioViewModel::criticalActiveChanged);
        QSignalSpy pulseActiveSpy(&viewModel, &SafetyScenarioViewModel::pulseActiveChanged);
        QSignalSpy acknowledgementAvailableSpy(&viewModel, &SafetyScenarioViewModel::acknowledgementAvailableChanged);

        delete service;

        QVERIFY(!viewModel.isActive());
        QVERIFY(!viewModel.presentationVisible());
        QVERIFY(viewModel.canStart());
        QCOMPARE(viewModel.title(), QStringLiteral("SAFETY LAB"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("FORWARD HAZARD SIMULATION"));
        QVERIFY(isNear(viewModel.riskProgress(), 0.0));
        QCOMPARE(viewModel.riskSegments(), 0);
        QVERIFY(isNear(viewModel.threatPosition(), 0.0));
        QVERIFY(!viewModel.advisoryActive());
        QVERIFY(!viewModel.criticalActive());
        QVERIFY(!viewModel.pulseActive());
        QVERIFY(!viewModel.acknowledgementAvailable());
        QCOMPARE(activeSpy.size(), 1);
        QCOMPARE(presentationVisibleSpy.size(), 1);
        QCOMPARE(canStartSpy.size(), 1);
        QCOMPARE(displaySpy.size(), 1);
        QCOMPARE(riskProgressSpy.size(), 1);
        QCOMPARE(riskSegmentsSpy.size(), 1);
        QCOMPARE(threatPositionSpy.size(), 1);
        QCOMPARE(criticalActiveSpy.size(), 1);
        QCOMPARE(pulseActiveSpy.size(), 1);
        QCOMPARE(acknowledgementAvailableSpy.size(), 1);

        viewModel.startDemo();
        QCOMPARE(activeSpy.size(), 1);
    }

    void naturalCompletionUpdatesTheViewModelAndReplayRestartsNormal()
    {
        MockSafetyScenarioService service;
        SafetyScenarioViewModel viewModel(&service);
        viewModel.setPresentationAllowed(true);

        QSignalSpy activeSpy(&viewModel, &SafetyScenarioViewModel::activeChanged);
        QSignalSpy presentationVisibleSpy(&viewModel, &SafetyScenarioViewModel::presentationVisibleChanged);
        QSignalSpy canStartSpy(&viewModel, &SafetyScenarioViewModel::canStartChanged);

        viewModel.startDemo();
        service.advance(72000);

        QVERIFY(!viewModel.isActive());
        QVERIFY(!viewModel.presentationVisible());
        QVERIFY(viewModel.canStart());
        QCOMPARE(viewModel.title(), QStringLiteral("SAFETY LAB"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("FORWARD HAZARD SIMULATION"));
        QCOMPARE(activeSpy.size(), 2);
        QCOMPARE(presentationVisibleSpy.size(), 2);
        QCOMPARE(canStartSpy.size(), 2);

        service.advance(MockSafetyScenarioService::updateIntervalMs());
        QCOMPARE(activeSpy.size(), 2);
        QCOMPARE(presentationVisibleSpy.size(), 2);
        QCOMPARE(canStartSpy.size(), 2);

        viewModel.replay();
        QVERIFY(viewModel.isActive());
        QVERIFY(viewModel.presentationVisible());
        QVERIFY(!viewModel.canStart());
        QCOMPARE(service.phase(), MockSafetyScenarioService::Phase::Normal);
        QCOMPARE(viewModel.title(), QStringLiteral("SAFETY LAB"));
        QCOMPARE(viewModel.instructionText(), QStringLiteral("FORWARD HAZARD SIMULATION"));
    }

    void effectivePropertyNotificationsAreNotDuplicated()
    {
        MockSafetyScenarioService service;
        SafetyScenarioViewModel viewModel(&service);
        viewModel.setPresentationAllowed(true);

        QSignalSpy activeSpy(&viewModel, &SafetyScenarioViewModel::activeChanged);
        QSignalSpy presentationVisibleSpy(&viewModel, &SafetyScenarioViewModel::presentationVisibleChanged);
        QSignalSpy canStartSpy(&viewModel, &SafetyScenarioViewModel::canStartChanged);
        QSignalSpy displaySpy(&viewModel, &SafetyScenarioViewModel::displayChanged);

        viewModel.startDemo();
        QCOMPARE(activeSpy.size(), 1);
        QCOMPARE(presentationVisibleSpy.size(), 1);
        QCOMPARE(canStartSpy.size(), 1);
        QCOMPARE(displaySpy.size(), 0);

        viewModel.startDemo();
        QCOMPARE(activeSpy.size(), 1);
        QCOMPARE(presentationVisibleSpy.size(), 1);
        QCOMPARE(canStartSpy.size(), 1);
        QCOMPARE(displaySpy.size(), 0);

        viewModel.setPresentationAllowed(false);
        QCOMPARE(activeSpy.size(), 2);
        QCOMPARE(presentationVisibleSpy.size(), 2);
        QCOMPARE(canStartSpy.size(), 1);
        QCOMPARE(displaySpy.size(), 0);

        viewModel.setPresentationAllowed(false);
        QCOMPARE(activeSpy.size(), 2);
        QCOMPARE(presentationVisibleSpy.size(), 2);
        QCOMPARE(canStartSpy.size(), 1);
        QCOMPARE(displaySpy.size(), 0);
    }
};

QTEST_MAIN(TestSafetyScenario)
#include "tst_safety_scenario.moc"
