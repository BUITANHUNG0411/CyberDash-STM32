#include "EncoderDriveViewModel.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace {
constexpr qint64 kMaximumElapsedMs = 100;
constexpr std::chrono::milliseconds kStaleTimeout{500};
constexpr double kMinimumMotion = 0.05;
constexpr double kStraightThreshold = 0.05;
constexpr double kGentleThreshold = 0.20;
constexpr double kMaximumResponseRatio = 0.75;
constexpr double kResponsePerSecond = 6.0;
constexpr double kMaximumLateralOffset = 0.28;
constexpr double kMaximumYawDegrees = 14.0;
constexpr double kMaximumRoadCurvature = 0.72;
constexpr double kHorizonY = 0.16;
constexpr double kNearY = 0.98;
constexpr double kHorizonHalfWidth = 0.08;
constexpr double kNearHalfWidth = 0.45;
constexpr double kHorizonCurvatureScale = 0.22;
constexpr double kNearCurvatureScale = 0.04;
constexpr double kThresholdTolerance = 1.0e-12;

bool differs(qreal current, qreal next)
{
    return !qFuzzyCompare(current + 1.0, next + 1.0);
}

double boundedCoordinate(double value)
{
    if (!std::isfinite(value)) {
        return 0.0;
    }
    return std::clamp(value, 0.0, 1.0);
}
}

EncoderDriveViewModel::EncoderDriveViewModel(QObject *parent)
    : QObject(parent)
    , m_staleTimer(new QTimer(this))
{
    m_staleTimer->setSingleShot(true);
    m_staleTimer->setInterval(kStaleTimeout);
    connect(m_staleTimer, &QTimer::timeout,
            this, &EncoderDriveViewModel::handleStaleTimeout);
    updateRoadPaths();
}

qreal EncoderDriveViewModel::forwardSpeed() const
{
    return m_forwardSpeed;
}

qreal EncoderDriveViewModel::vehicleLateralOffset() const
{
    return m_vehicleLateralOffset;
}

qreal EncoderDriveViewModel::vehicleYawDegrees() const
{
    return m_vehicleYawDegrees;
}

qreal EncoderDriveViewModel::roadCurvature() const
{
    return m_roadCurvature;
}

QString EncoderDriveViewModel::roadPath() const
{
    return m_roadPath;
}

QString EncoderDriveViewModel::roadEdgePath() const
{
    return m_roadEdgePath;
}

EncoderDriveViewModel::TurnState EncoderDriveViewModel::turnState() const
{
    return m_turnState;
}

void EncoderDriveViewModel::updateWheelMotion(double leftMotion,
                                              double rightMotion,
                                              qint64 elapsedMs)
{
    if (elapsedMs <= 0) {
        return;
    }

    if (!std::isfinite(leftMotion) || !std::isfinite(rightMotion)) {
        leftMotion = 0.0;
        rightMotion = 0.0;
    }

    leftMotion = (std::max)(leftMotion, 0.0);
    rightMotion = (std::max)(rightMotion, 0.0);
    const double normalizationScale =
        (std::max)({leftMotion, rightMotion, 1.0});
    leftMotion = std::clamp(
        leftMotion / normalizationScale, 0.0, 1.0);
    rightMotion = std::clamp(
        rightMotion / normalizationScale, 0.0, 1.0);

    const double meanMotion = (leftMotion + rightMotion) / 2.0;
    const double signedDifference = rightMotion - leftMotion;
    const double ratio = std::abs(signedDifference)
        / (std::max)(meanMotion, kMinimumMotion);
    const double speed =
        meanMotion >= kMinimumMotion ? meanMotion : 0.0;
    const qint64 boundedElapsedMs =
        (std::min)(elapsedMs, kMaximumElapsedMs);
    const double elapsedSeconds =
        static_cast<double>(boundedElapsedMs) / 1000.0;
    const double alpha =
        (std::min)(1.0, elapsedSeconds * kResponsePerSecond);

    const TurnState nextTurnState =
        speed > 0.0 ? classifyTurn(ratio, signedDifference) : Straight;
    const double response = nextTurnState == Straight
        ? 0.0
        : std::clamp(
            (ratio - kStraightThreshold)
                / (kMaximumResponseRatio - kStraightThreshold),
            0.0,
            1.0);
    const double visualDirection = signedDifference > 0.0 ? -1.0 : 1.0;
    const double targetLateralOffset =
        visualDirection * response * kMaximumLateralOffset;
    const double targetYawDegrees =
        visualDirection * response * kMaximumYawDegrees;
    const double targetRoadCurvature =
        visualDirection * response * kMaximumRoadCurvature;

    const qreal nextSpeed = static_cast<qreal>(speed);
    const qreal nextLateralOffset = static_cast<qreal>(std::clamp(
        static_cast<double>(m_vehicleLateralOffset)
            + (targetLateralOffset
               - static_cast<double>(m_vehicleLateralOffset)) * alpha,
        -kMaximumLateralOffset,
        kMaximumLateralOffset));
    const qreal nextYawDegrees = static_cast<qreal>(std::clamp(
        static_cast<double>(m_vehicleYawDegrees)
            + (targetYawDegrees
               - static_cast<double>(m_vehicleYawDegrees)) * alpha,
        -kMaximumYawDegrees,
        kMaximumYawDegrees));
    const qreal nextRoadCurvature = static_cast<qreal>(std::clamp(
        static_cast<double>(m_roadCurvature)
            + (targetRoadCurvature
               - static_cast<double>(m_roadCurvature)) * alpha,
        -kMaximumRoadCurvature,
        kMaximumRoadCurvature));

    if (differs(m_forwardSpeed, nextSpeed)) {
        m_forwardSpeed = nextSpeed;
        emit forwardSpeedChanged();
    }
    if (differs(m_vehicleLateralOffset, nextLateralOffset)) {
        m_vehicleLateralOffset = nextLateralOffset;
        emit vehicleLateralOffsetChanged();
    }
    if (differs(m_vehicleYawDegrees, nextYawDegrees)) {
        m_vehicleYawDegrees = nextYawDegrees;
        emit vehicleYawDegreesChanged();
    }
    bool roadGeometryChanged = false;
    if (differs(m_roadCurvature, nextRoadCurvature)) {
        m_roadCurvature = nextRoadCurvature;
        roadGeometryChanged = true;
        emit roadCurvatureChanged();
    }
    if (m_turnState != nextTurnState) {
        m_turnState = nextTurnState;
        emit turnStateChanged();
    }
    if (roadGeometryChanged) {
        updateRoadPaths();
    }

    m_staleTimer->start();
}

void EncoderDriveViewModel::handleStaleTimeout()
{
    if (!qFuzzyIsNull(m_forwardSpeed)) {
        m_forwardSpeed = 0.0;
        emit forwardSpeedChanged();
    }
    if (!qFuzzyIsNull(m_vehicleYawDegrees)) {
        m_vehicleYawDegrees = 0.0;
        emit vehicleYawDegreesChanged();
    }

    bool roadGeometryChanged = false;
    if (!qFuzzyIsNull(m_roadCurvature)) {
        m_roadCurvature = 0.0;
        roadGeometryChanged = true;
        emit roadCurvatureChanged();
    }
    if (m_turnState != Straight) {
        m_turnState = Straight;
        emit turnStateChanged();
    }
    if (roadGeometryChanged) {
        updateRoadPaths();
    }
}

void EncoderDriveViewModel::updateRoadPaths()
{
    const double curvature = std::clamp(
        static_cast<double>(m_roadCurvature),
        -kMaximumRoadCurvature,
        kMaximumRoadCurvature);
    const double horizonCenter =
        0.5 + curvature * kHorizonCurvatureScale;
    const double nearCenter =
        0.5 + curvature * kNearCurvatureScale;
    const double horizonLeftX =
        boundedCoordinate(horizonCenter - kHorizonHalfWidth);
    const double horizonRightX =
        boundedCoordinate(horizonCenter + kHorizonHalfWidth);
    const double nearLeftX =
        boundedCoordinate(nearCenter - kNearHalfWidth);
    const double nearRightX =
        boundedCoordinate(nearCenter + kNearHalfWidth);
    const double horizonY = boundedCoordinate(kHorizonY);
    const double nearY = boundedCoordinate(kNearY);

    const QString nextRoadPath = formatRoadPath(
        horizonLeftX,
        horizonY,
        nearLeftX,
        nearY,
        nearRightX,
        horizonRightX);
    const QString nextRoadEdgePath = formatRoadEdgePath(
        horizonLeftX,
        horizonY,
        nearLeftX,
        nearY,
        nearRightX,
        horizonRightX);

    if (m_roadPath != nextRoadPath) {
        m_roadPath = nextRoadPath;
        emit roadPathChanged();
    }
    if (m_roadEdgePath != nextRoadEdgePath) {
        m_roadEdgePath = nextRoadEdgePath;
        emit roadEdgePathChanged();
    }
}

EncoderDriveViewModel::TurnState EncoderDriveViewModel::classifyTurn(
    double ratio,
    double signedDifference)
{
    if (ratio + kThresholdTolerance < kStraightThreshold
        || qFuzzyIsNull(signedDifference)) {
        return Straight;
    }

    const bool isGentle =
        ratio <= kGentleThreshold + kThresholdTolerance;
    if (signedDifference > 0.0) {
        return isGentle ? GentleLeft : TurningLeft;
    }
    return isGentle ? GentleRight : TurningRight;
}

QString EncoderDriveViewModel::formatRoadPath(double horizonLeftX,
                                              double horizonY,
                                              double nearLeftX,
                                              double nearY,
                                              double nearRightX,
                                              double horizonRightX)
{
    return QString::asprintf(
        "M %.3f %.3f L %.3f %.3f L %.3f %.3f L %.3f %.3f Z",
        boundedCoordinate(horizonLeftX),
        boundedCoordinate(horizonY),
        boundedCoordinate(nearLeftX),
        boundedCoordinate(nearY),
        boundedCoordinate(nearRightX),
        boundedCoordinate(nearY),
        boundedCoordinate(horizonRightX),
        boundedCoordinate(horizonY));
}

QString EncoderDriveViewModel::formatRoadEdgePath(double horizonLeftX,
                                                  double horizonY,
                                                  double nearLeftX,
                                                  double nearY,
                                                  double nearRightX,
                                                  double horizonRightX)
{
    return QString::asprintf(
        "M %.3f %.3f L %.3f %.3f M %.3f %.3f L %.3f %.3f",
        boundedCoordinate(horizonLeftX),
        boundedCoordinate(horizonY),
        boundedCoordinate(nearLeftX),
        boundedCoordinate(nearY),
        boundedCoordinate(horizonRightX),
        boundedCoordinate(horizonY),
        boundedCoordinate(nearRightX),
        boundedCoordinate(nearY));
}
