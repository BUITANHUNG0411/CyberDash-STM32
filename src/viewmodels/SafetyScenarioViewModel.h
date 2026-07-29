#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class MockSafetyScenarioService;

class SafetyScenarioViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool presentationVisible READ presentationVisible NOTIFY presentationVisibleChanged)
    Q_PROPERTY(bool canStart READ canStart NOTIFY canStartChanged)
    Q_PROPERTY(QString title READ title NOTIFY displayChanged)
    Q_PROPERTY(QString instructionText READ instructionText NOTIFY displayChanged)
    Q_PROPERTY(QString disclaimerText READ disclaimerText CONSTANT)
    Q_PROPERTY(qreal riskProgress READ riskProgress NOTIFY riskProgressChanged)
    Q_PROPERTY(int riskSegments READ riskSegments NOTIFY riskSegmentsChanged)
    Q_PROPERTY(qreal threatPosition READ threatPosition NOTIFY threatPositionChanged)
    Q_PROPERTY(bool advisoryActive READ advisoryActive NOTIFY advisoryActiveChanged)
    Q_PROPERTY(bool criticalActive READ criticalActive NOTIFY criticalActiveChanged)
    Q_PROPERTY(bool pulseActive READ pulseActive NOTIFY pulseActiveChanged)
    Q_PROPERTY(bool acknowledgementAvailable READ acknowledgementAvailable NOTIFY acknowledgementAvailableChanged)

public:
    explicit SafetyScenarioViewModel(MockSafetyScenarioService *service, QObject *parent = nullptr);

    bool isActive() const;
    bool presentationVisible() const;
    bool canStart() const;
    QString title() const;
    QString instructionText() const;
    QString disclaimerText() const;
    qreal riskProgress() const;
    int riskSegments() const;
    qreal threatPosition() const;
    bool advisoryActive() const;
    bool criticalActive() const;
    bool pulseActive() const;
    bool acknowledgementAvailable() const;

    void setPresentationAllowed(bool allowed);

    Q_INVOKABLE void startDemo();
    Q_INVOKABLE void replay();
    Q_INVOKABLE void acknowledge();
    Q_INVOKABLE void stopDemo();

signals:
    void activeChanged();
    void presentationVisibleChanged();
    void canStartChanged();
    void displayChanged();
    void riskProgressChanged();
    void riskSegmentsChanged();
    void threatPositionChanged();
    void advisoryActiveChanged();
    void criticalActiveChanged();
    void pulseActiveChanged();
    void acknowledgementAvailableChanged();

private:
    void synchronizeFromService();

    QPointer<MockSafetyScenarioService> m_service;
    bool m_presentationAllowed = false;
    bool m_active = false;
    bool m_presentationVisible = false;
    bool m_canStart = false;
    QString m_title = QStringLiteral("SAFETY LAB");
    QString m_instructionText = QStringLiteral("FORWARD HAZARD SIMULATION");
    qreal m_riskProgress = 0.0;
    int m_riskSegments = 0;
    qreal m_threatPosition = 0.0;
    bool m_advisoryActive = false;
    bool m_criticalActive = false;
    bool m_pulseActive = false;
    bool m_acknowledgementAvailable = false;
};
