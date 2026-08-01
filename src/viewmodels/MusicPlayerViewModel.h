#pragma once

#include <QAbstractListModel>
#include <QThread>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QSettings>
#include <qqml.h>
#include "../services/MusicScanner.h"

// Namespace-scoped enums exposed to QML via Q_NAMESPACE.
namespace MusicEnums {
Q_NAMESPACE
QML_ELEMENT
enum class PlaybackState { Stopped, Playing, Paused, Loading };
Q_ENUM_NS(PlaybackState)

enum class RepeatMode { Off, One, All };
Q_ENUM_NS(RepeatMode)
}

class MusicPlayerViewModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentCoverArt READ currentCoverArt NOTIFY currentTrackChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)

    // ==== New properties (Music Player Upgrade - Step 1) ====
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool shuffleMode READ shuffleMode WRITE setShuffleMode NOTIFY shuffleModeChanged)
    Q_PROPERTY(MusicEnums::PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(MusicEnums::RepeatMode repeatMode READ repeatMode WRITE setRepeatMode NOTIFY repeatModeChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 positionMs READ positionMs NOTIFY positionChanged)
    Q_PROPERTY(bool scrubberDragging READ scrubberDragging NOTIFY scrubberStateChanged)
    Q_PROPERTY(float scrubberRatio READ scrubberRatio NOTIFY scrubberStateChanged)

public:
    enum SongRoles {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        AlbumRole,
        FilePathRole,
        Color1Role,
        Color2Role,
        CoverArtRole
    };

    explicit MusicPlayerViewModel(QObject *parent = nullptr,
                                  bool multimediaEnabled = true,
                                  bool persistenceEnabled = true);
    ~MusicPlayerViewModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const;
    void setCurrentIndex(int index);
    QString currentTitle() const;
    QString currentArtist() const;
    QString currentCoverArt() const;

    bool isPlaying() const;
    float progress() const;
    bool isScanning() const;

    // ==== New getters (Step 1) ====
    float volume() const;
    bool isLoading() const;
    QString lastError() const;
    bool shuffleMode() const;
    MusicEnums::PlaybackState playbackState() const;
    MusicEnums::RepeatMode repeatMode() const;
    qint64 duration() const;
    qint64 positionMs() const;
    bool scrubberDragging() const;
    float scrubberRatio() const;

public slots:
    void play(int index = -1);
    void pause();
    void togglePlayPause();
    void next();
    void prev();
    void scanLibrary();

    // ==== New Q_INVOKABLEs / slots (Step 1) ====
    Q_INVOKABLE void seek(float ratio);
    Q_INVOKABLE void seekMs(qint64 ms);
    Q_INVOKABLE void toggleShuffle();
    Q_INVOKABLE void cycleRepeat();
    Q_INVOKABLE void clearError();
    Q_INVOKABLE void saveResume();
    Q_INVOKABLE void beginScrub(qreal position, qreal width);
    Q_INVOKABLE void updateScrub(qreal position, qreal width);
    Q_INVOKABLE void endScrub();
    Q_INVOKABLE void setVolumeFromPosition(qreal position, qreal width);

    void setVolume(float value);
    void setShuffleMode(bool enabled);
    void setRepeatMode(MusicEnums::RepeatMode mode);

private slots:
    void onSongFound(const SongData& song);
    void onScanFinished();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);

    // ==== New internal slots (Step 1) ====
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);
    void onPeriodicSaveTimeout();

signals:
    void currentIndexChanged();
    void currentTrackChanged();
    void isPlayingChanged();
    void progressChanged();
    void isScanningChanged();
    void requestScan(const QString& path);

    // ==== New signals (Step 1) ====
    void volumeChanged();
    void isLoadingChanged();
    void shuffleModeChanged();
    void lastErrorChanged();
    void repeatModeChanged();
    void playbackStateChanged();
    void durationChanged();
    void positionChanged();
    void playbackError(const QString& message);
    void scrubberStateChanged();

private:
    bool ensurePlayer();
    void updatePlaybackState();
    void saveResumeNow();
    void scheduleResumeSave();
    int nextShuffleIndex() const;

    QList<SongData> m_songs;
    int m_currentIndex = -1;
    bool m_isScanning = false;
    float m_progress = 0.0f;

    // ==== New private state (Step 1) ====
    float m_volume = 1.0f;
    bool m_isLoading = false;
    bool m_shuffleMode = false;
    QString m_lastError = "";
    MusicEnums::RepeatMode m_repeatMode = MusicEnums::RepeatMode::Off;
    MusicEnums::PlaybackState m_playbackState = MusicEnums::PlaybackState::Stopped;
    qint64 m_duration = 0;
    qint64 m_positionMs = 0;
    qint64 m_lastSavedPosition = 0;
    int m_lastIndex = -1;
    qint64 m_lastPos = 0;
    bool m_resumePending = false;
    bool m_persistenceEnabled = true;
    bool m_multimediaEnabled = true;
    bool m_scrubberDragging = false;
    float m_scrubberRatio = 0.0f;

    QThread m_scannerThread;
    MusicScanner* m_scanner;

    QMediaPlayer* m_player = nullptr;
    QAudioOutput* m_audioOutput = nullptr;

    QTimer* m_saveTimer;
    QSettings m_settings;
};
