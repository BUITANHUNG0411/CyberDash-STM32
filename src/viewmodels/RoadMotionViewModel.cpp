#include "RoadMotionViewModel.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace {
constexpr int kSegmentCount = 24;
constexpr qint64 kMaximumElapsedMs = 100;
constexpr std::chrono::milliseconds kStaleTimeout{500};
constexpr double kWheelTrack = 1.0;
constexpr double kMinimumForwardSpeed = 0.05;
constexpr double kMaximumWheelSpeed = 1.0;
constexpr double kMaximumCurvature = 0.75;
constexpr double kScrollScale = 0.45;
constexpr double kCurvatureResponsePerSecond = 4.0;
constexpr double kHorizonY = 0.14;
constexpr double kRoadHeight = 0.82;
constexpr double kMinimumX = -0.10;
constexpr double kMaximumX = 1.10;
constexpr double kLateralOffsetScale = 0.20;
constexpr double kMaximumLateralOffset = 0.35;

const QList<int> kGeometryRoles{
    RoadMotionViewModel::LeftNearXRole,
    RoadMotionViewModel::RightNearXRole,
    RoadMotionViewModel::LeftFarXRole,
    RoadMotionViewModel::RightFarXRole,
    RoadMotionViewModel::NearYRole,
    RoadMotionViewModel::FarYRole,
    RoadMotionViewModel::CenterNearXRole,
    RoadMotionViewModel::CenterFarXRole,
    RoadMotionViewModel::CenterNearYRole,
    RoadMotionViewModel::CenterFarYRole,
    RoadMotionViewModel::CenterLineVisibleRole,
    RoadMotionViewModel::SegmentOpacityRole,
    RoadMotionViewModel::SegmentDepthRole
};
}

RoadMotionViewModel::RoadMotionViewModel(QObject *parent)
    : QAbstractListModel(parent)
{
    initializeSegments();
    m_staleTimer.setSingleShot(true);
    m_staleTimer.setInterval(kStaleTimeout);
    connect(&m_staleTimer, &QTimer::timeout,
            this, &RoadMotionViewModel::handleStaleTimeout);
}

int RoadMotionViewModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_segments.size());
}

QVariant RoadMotionViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || index.row() >= m_segments.size()) {
        return {};
    }

    return roleData(m_segments.at(index.row()), index.row(), role);
}

QHash<int, QByteArray> RoadMotionViewModel::roleNames() const
{
    return {
        {LeftNearXRole, "leftNearX"},
        {RightNearXRole, "rightNearX"},
        {LeftFarXRole, "leftFarX"},
        {RightFarXRole, "rightFarX"},
        {NearYRole, "nearY"},
        {FarYRole, "farY"},
        {CenterNearXRole, "centerNearX"},
        {CenterFarXRole, "centerFarX"},
        {CenterNearYRole, "centerNearY"},
        {CenterFarYRole, "centerFarY"},
        {CenterLineVisibleRole, "centerLineVisible"},
        {SegmentOpacityRole, "segmentOpacity"},
        {SegmentDepthRole, "segmentDepth"}
    };
}

qreal RoadMotionViewModel::forwardSpeed() const
{
    return m_forwardSpeed;
}

qreal RoadMotionViewModel::curvature() const
{
    return m_curvature;
}

void RoadMotionViewModel::updateWheelMotion(double leftWheelSpeed,
                                            double rightWheelSpeed,
                                            qint64 elapsedMs)
{
    if (elapsedMs <= 0) {
        return;
    }

    if (!std::isfinite(leftWheelSpeed)
        || !std::isfinite(rightWheelSpeed)) {
        leftWheelSpeed = 0.0;
        rightWheelSpeed = 0.0;
    }

    leftWheelSpeed = std::clamp(
        leftWheelSpeed, 0.0, kMaximumWheelSpeed);
    rightWheelSpeed = std::clamp(
        rightWheelSpeed, 0.0, kMaximumWheelSpeed);

    const qint64 clampedElapsedMs =
        (std::min)(elapsedMs, kMaximumElapsedMs);
    const double elapsedSeconds =
        static_cast<double>(clampedElapsedMs) / 1000.0;
    const double rawSpeed =
        leftWheelSpeed / 2.0 + rightWheelSpeed / 2.0;
    const double speed =
        rawSpeed >= kMinimumForwardSpeed ? rawSpeed : 0.0;

    double targetCurvature = 0.0;
    if (speed >= kMinimumForwardSpeed) {
        const double turnRate =
            (rightWheelSpeed - leftWheelSpeed) / kWheelTrack;
        targetCurvature = std::clamp(
            turnRate / (std::max)(speed, kMinimumForwardSpeed),
            -kMaximumCurvature,
            kMaximumCurvature);
    }

    const double alpha = (std::min)(
        1.0,
        elapsedSeconds * kCurvatureResponsePerSecond);
    const double smoothedCurvature =
        static_cast<double>(m_curvature)
        + (targetCurvature - static_cast<double>(m_curvature)) * alpha;

    bool geometryChanged =
    updatePublishedMotion(speed, smoothedCurvature);
    if (speed > 0.0) {
        m_lateralOffset = std::clamp(
            static_cast<double>(m_lateralOffset)
                - static_cast<double>(m_curvature)
                    * speed * elapsedSeconds * kLateralOffsetScale,
            -kMaximumLateralOffset,
            kMaximumLateralOffset);
    }
    if (speed > 0.0) {
        advanceSegments(elapsedSeconds);
        geometryChanged = true;
    }
    if (geometryChanged) {
        publishGeometry();
    }
    m_staleTimer.start();
}

void RoadMotionViewModel::resetRoad()
{
    m_staleTimer.stop();
    updatePublishedMotion(0.0, 0.0);
    m_lateralOffset = 0.0;
    initializeSegments();
    publishGeometry();
}

void RoadMotionViewModel::handleStaleTimeout()
{
    if (qFuzzyIsNull(m_forwardSpeed)) {
        return;
    }

    m_forwardSpeed = 0.0;
    emit forwardSpeedChanged();
}

void RoadMotionViewModel::initializeSegments()
{
    m_segments.clear();
    m_segments.reserve(kSegmentCount);
    for (int row = 0; row < kSegmentCount; ++row) {
        m_segments.push_back({
            static_cast<double>(row)
                / static_cast<double>(kSegmentCount),
            0
        });
    }
}

bool RoadMotionViewModel::updatePublishedMotion(double speed,
                                                double curvature)
{
    const qreal effectiveSpeed = static_cast<qreal>(speed);
    const qreal effectiveCurvature = static_cast<qreal>(curvature);
    bool didCurvatureChange = false;

    if (!qFuzzyCompare(m_forwardSpeed + 1.0, effectiveSpeed + 1.0)) {
        m_forwardSpeed = effectiveSpeed;
        emit forwardSpeedChanged();
    }
    if (!qFuzzyCompare(m_curvature + 1.0, effectiveCurvature + 1.0)) {
        m_curvature = effectiveCurvature;
        didCurvatureChange = true;
        emit curvatureChanged();
    }
    return didCurvatureChange;
}

void RoadMotionViewModel::advanceSegments(double elapsedSeconds)
{
    const double depthDelta =
        static_cast<double>(m_forwardSpeed) * elapsedSeconds * kScrollScale;
    for (Segment &segment : m_segments) {
        const double advancedDepth = segment.depth + depthDelta;
        const double wrapCount = std::floor(advancedDepth);
        segment.depth = std::fmod(advancedDepth, 1.0);
        if (wrapCount > 0.0) {
            segment.generation += static_cast<quint64>(wrapCount);
        }
    }
}

QVariant RoadMotionViewModel::roleData(const Segment &segment,
                                       int row,
                                       int role) const
{
    const double spacing = 1.0 / static_cast<double>(kSegmentCount);
    const double farDepth = std::clamp(segment.depth, 0.0, 1.0);
    const double nearDepth = (std::min)(1.0, farDepth + spacing);
    const double farCenter = projectedCenter(farDepth);
    const double nearCenter = projectedCenter(nearDepth);
    const double farHalfWidth = projectedHalfWidth(farDepth);
    const double nearHalfWidth = projectedHalfWidth(nearDepth);

    switch (role) {
    case LeftNearXRole:
        return std::clamp(
            nearCenter - nearHalfWidth, kMinimumX, kMaximumX);
    case RightNearXRole:
        return std::clamp(
            nearCenter + nearHalfWidth, kMinimumX, kMaximumX);
    case LeftFarXRole:
        return std::clamp(
            farCenter - farHalfWidth, kMinimumX, kMaximumX);
    case RightFarXRole:
        return std::clamp(
            farCenter + farHalfWidth, kMinimumX, kMaximumX);
    case NearYRole:
    case CenterNearYRole:
        return projectedY(nearDepth);
    case FarYRole:
    case CenterFarYRole:
        return projectedY(farDepth);
    case CenterNearXRole:
        return nearCenter;
    case CenterFarXRole:
        return farCenter;
    case CenterLineVisibleRole: {
        const quint64 absoluteOrdinal =
            static_cast<quint64>(row)
            + segment.generation * static_cast<quint64>(kSegmentCount);
        return absoluteOrdinal % 2U == 0U;
    }
    case SegmentOpacityRole:
        return 0.20 + farDepth * 0.80;
    case SegmentDepthRole:
        return farDepth;
    }
    return {};
}

double RoadMotionViewModel::projectedY(double depth) const
{
    return kHorizonY
        + std::pow(std::clamp(depth, 0.0, 1.0), 1.7) * kRoadHeight;
}

double RoadMotionViewModel::projectedCenter(double depth) const
{
    const double clampedDepth = std::clamp(depth, 0.0, 1.0);
    return std::clamp(
        0.5 + static_cast<double>(m_lateralOffset) * clampedDepth
            - static_cast<double>(m_curvature)
                * clampedDepth * clampedDepth * 0.30,
        kMinimumX,
        kMaximumX);
}

double RoadMotionViewModel::projectedHalfWidth(double depth) const
{
    const double clampedDepth = std::clamp(depth, 0.0, 1.0);
    return 0.04 + clampedDepth * clampedDepth * 0.44;
}

void RoadMotionViewModel::publishGeometry()
{
    if (m_segments.isEmpty()) {
        return;
    }

    emit dataChanged(
        index(0, 0),
        index(static_cast<int>(m_segments.size()) - 1, 0),
        kGeometryRoles);
}
