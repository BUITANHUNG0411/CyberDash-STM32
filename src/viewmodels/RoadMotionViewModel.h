#pragma once

#include <QAbstractListModel>
#include <QTimer>
#include <QVector>

class RoadMotionViewModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(qreal forwardSpeed READ forwardSpeed NOTIFY forwardSpeedChanged)
    Q_PROPERTY(qreal curvature READ curvature NOTIFY curvatureChanged)

public:
    enum Role : int {
        LeftNearXRole = Qt::UserRole + 1,
        RightNearXRole,
        LeftFarXRole,
        RightFarXRole,
        NearYRole,
        FarYRole,
        CenterNearXRole,
        CenterFarXRole,
        CenterNearYRole,
        CenterFarYRole,
        CenterLineVisibleRole,
        SegmentOpacityRole,
        SegmentDepthRole
    };
    Q_ENUM(Role)

    explicit RoadMotionViewModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    qreal forwardSpeed() const;
    qreal curvature() const;

public slots:
    void updateWheelMotion(double leftWheelSpeed,
                           double rightWheelSpeed,
                           qint64 elapsedMs);
    void resetRoad();

signals:
    void forwardSpeedChanged();
    void curvatureChanged();

private slots:
    void handleStaleTimeout();

private:
    struct Segment {
        double depth = 0.0;
        quint64 generation = 0;
    };

    void initializeSegments();
    bool updatePublishedMotion(double speed, double curvature);
    void advanceSegments(double elapsedSeconds);
    QVariant roleData(const Segment &segment, int row, int role) const;
    double projectedY(double depth) const;
    double projectedCenter(double depth) const;
    double projectedHalfWidth(double depth) const;
    void publishGeometry();

    QVector<Segment> m_segments;
    QTimer m_staleTimer;
    qreal m_forwardSpeed = 0.0;
    qreal m_curvature = 0.0;
};
