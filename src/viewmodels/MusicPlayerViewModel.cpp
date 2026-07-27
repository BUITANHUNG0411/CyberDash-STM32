#include "MusicPlayerViewModel.h"
#include <QStandardPaths>
#include <QDebug>
#include <QUrl>
#include <QRandomGenerator>
#include <algorithm>

namespace {
constexpr int kSaveIntervalMs = 1000;
constexpr qint64 kSavePositionDeltaMs = 250;
constexpr char kOrgName[] = "QtStmAutomotiveSimulator";
constexpr char kAppName[] = "QtStmAutomotiveSimulator";
}

MusicPlayerViewModel::MusicPlayerViewModel(QObject *parent, bool multimediaEnabled)
    : QAbstractListModel(parent), m_scanner(new MusicScanner()),
      m_settings(QSettings::IniFormat, QSettings::UserScope, kOrgName, kAppName)
{
    if (multimediaEnabled) {
        m_player = new QMediaPlayer(this);
        m_audioOutput = new QAudioOutput(this);
        m_player->setAudioOutput(m_audioOutput);

        connect(m_player, &QMediaPlayer::playingChanged, this, &MusicPlayerViewModel::isPlayingChanged);
        connect(m_player, &QMediaPlayer::positionChanged, this, &MusicPlayerViewModel::onPositionChanged);
        connect(m_player, &QMediaPlayer::durationChanged, this, &MusicPlayerViewModel::onDurationChanged);
        connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MusicPlayerViewModel::onMediaStatusChanged);
        connect(m_player, &QMediaPlayer::errorOccurred, this, &MusicPlayerViewModel::onErrorOccurred);
        m_audioOutput->setVolume(m_volume);
    }

    // Throttled resume save (1-second single-shot)
    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(kSaveIntervalMs);
    connect(m_saveTimer, &QTimer::timeout, this, &MusicPlayerViewModel::onPeriodicSaveTimeout);

    // Restore resume point
    m_lastIndex = m_settings.value("music/lastIndex", -1).toInt();
    m_lastPos = m_settings.value("music/lastPositionMs", 0).toLongLong();

    // Setup Scanner Thread
    m_scanner->moveToThread(&m_scannerThread);

    connect(this, &MusicPlayerViewModel::requestScan, m_scanner, &MusicScanner::scanLibrary);
    connect(m_scanner, &MusicScanner::songFound, this, &MusicPlayerViewModel::onSongFound);
    connect(m_scanner, &MusicScanner::scanFinished, this, &MusicPlayerViewModel::onScanFinished);
    connect(&m_scannerThread, &QThread::finished, m_scanner, &QObject::deleteLater);

    m_scannerThread.start();
}

MusicPlayerViewModel::~MusicPlayerViewModel()
{
    saveResumeNow();
    m_scannerThread.requestInterruption();
    m_scannerThread.quit();
    m_scannerThread.wait();
}

int MusicPlayerViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(m_songs.count());
}

QVariant MusicPlayerViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_songs.count())
        return QVariant();

    const SongData &song = m_songs[index.row()];
    switch (role) {
        case TitleRole: return song.title;
        case ArtistRole: return song.artist;
        case FilePathRole: return song.filePath;
        case Color1Role: return song.color1;
        case Color2Role: return song.color2;
        case CoverArtRole: return song.coverArt;
    }
    return QVariant();
}

QHash<int, QByteArray> MusicPlayerViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    roles[ArtistRole] = "artist";
    roles[FilePathRole] = "filePath";
    roles[Color1Role] = "color1";
    roles[Color2Role] = "color2";
    roles[CoverArtRole] = "coverArt";
    return roles;
}

int MusicPlayerViewModel::currentIndex() const
{
    return m_currentIndex;
}

void MusicPlayerViewModel::setCurrentIndex(int index)
{
    if (m_currentIndex == index) return;
    if (index >= 0 && index < m_songs.count()) {
        m_currentIndex = index;
        emit currentIndexChanged();

        // Reset progress when changing track
        m_progress = 0.0f;
        emit progressChanged();

        if (m_player) {
            m_player->setSource(QUrl::fromLocalFile(m_songs[m_currentIndex].filePath));
            m_player->play();
        }
    }
}

bool MusicPlayerViewModel::isPlaying() const
{
    return m_player && m_player->isPlaying();
}

float MusicPlayerViewModel::progress() const
{
    return m_progress;
}

bool MusicPlayerViewModel::isScanning() const
{
    return m_isScanning;
}

// ==== New getters (Step 1) ====
float MusicPlayerViewModel::volume() const
{
    return m_volume;
}

bool MusicPlayerViewModel::isLoading() const
{
    return m_isLoading;
}

QString MusicPlayerViewModel::lastError() const
{
    return m_lastError;
}

bool MusicPlayerViewModel::shuffleMode() const
{
    return m_shuffleMode;
}

MusicEnums::PlaybackState MusicPlayerViewModel::playbackState() const
{
    return m_playbackState;
}

MusicEnums::RepeatMode MusicPlayerViewModel::repeatMode() const
{
    return m_repeatMode;
}

qint64 MusicPlayerViewModel::duration() const
{
    return m_duration;
}

qint64 MusicPlayerViewModel::positionMs() const
{
    return m_positionMs;
}

bool MusicPlayerViewModel::scrubberDragging() const
{
    return m_scrubberDragging;
}

float MusicPlayerViewModel::scrubberRatio() const
{
    return m_scrubberRatio;
}

// ==== New setters / invokables (Step 1) ====
void MusicPlayerViewModel::setVolume(float value)
{
    float clamped = std::clamp(value, 0.0f, 1.0f);
    if (m_volume == clamped) return;
    m_volume = clamped;
    if (m_audioOutput)
        m_audioOutput->setVolume(m_volume);
    emit volumeChanged();
}

void MusicPlayerViewModel::setShuffleMode(bool enabled)
{
    if (m_shuffleMode == enabled) return;
    m_shuffleMode = enabled;
    emit shuffleModeChanged();
}

void MusicPlayerViewModel::toggleShuffle()
{
    setShuffleMode(!m_shuffleMode);
}

void MusicPlayerViewModel::setRepeatMode(MusicEnums::RepeatMode mode)
{
    if (m_repeatMode == mode) return;
    m_repeatMode = mode;
    emit repeatModeChanged();
    updatePlaybackState();
}

void MusicPlayerViewModel::cycleRepeat()
{
    MusicEnums::RepeatMode next;
    switch (m_repeatMode) {
        case MusicEnums::RepeatMode::Off: next = MusicEnums::RepeatMode::One; break;
        case MusicEnums::RepeatMode::One: next = MusicEnums::RepeatMode::All; break;
        case MusicEnums::RepeatMode::All: next = MusicEnums::RepeatMode::Off; break;
        default: next = MusicEnums::RepeatMode::Off; break;
    }
    setRepeatMode(next);
}

void MusicPlayerViewModel::seek(float ratio)
{
    if (!m_player || m_player->duration() <= 0) return; // no-op when no duration
    float clamped = std::clamp(ratio, 0.0f, 1.0f);
    m_player->setPosition(static_cast<qint64>(static_cast<float>(m_player->duration()) * clamped));
}

void MusicPlayerViewModel::seekMs(qint64 ms)
{
    if (!m_player || m_player->duration() <= 0) return;
    qint64 clamped = std::clamp(ms, static_cast<qint64>(0), m_player->duration());
    m_player->setPosition(clamped);
}

void MusicPlayerViewModel::saveResume()
{
    saveResumeNow();
}

void MusicPlayerViewModel::clearError()
{
    if (m_lastError != "") {
        m_lastError = "";
        emit lastErrorChanged();
    }
}

void MusicPlayerViewModel::beginScrub(qreal position, qreal width)
{
    if (!m_scrubberDragging) {
        m_scrubberDragging = true;
        emit scrubberStateChanged();
    }
    updateScrub(position, width);
}

void MusicPlayerViewModel::updateScrub(qreal position, qreal width)
{
    if (width <= 0.0)
        return;

    const float ratio = std::clamp(static_cast<float>(position / width), 0.0f, 1.0f);
    if (!qFuzzyCompare(m_scrubberRatio, ratio)) {
        m_scrubberRatio = ratio;
        emit scrubberStateChanged();
    }
    seek(ratio);
}

void MusicPlayerViewModel::endScrub()
{
    if (!m_scrubberDragging)
        return;

    m_scrubberDragging = false;
    emit scrubberStateChanged();
}

// ==== Existing slots ====
void MusicPlayerViewModel::play(int index)
{
    if (index != -1 && index != m_currentIndex) {
        setCurrentIndex(index);
    } else if (m_player && m_currentIndex >= 0 && m_currentIndex < m_songs.count()) {
        m_player->play();
    }
}

void MusicPlayerViewModel::pause()
{
    if (m_player)
        m_player->pause();
}

void MusicPlayerViewModel::togglePlayPause()
{
    if (!m_player)
        return;

    if (m_player->isPlaying()) {
        m_player->pause();
    } else {
        if (m_currentIndex >= 0 && m_currentIndex < m_songs.count()) {
            m_player->play();
        } else if (!m_songs.isEmpty()) {
            setCurrentIndex(0);
        }
    }
}

void MusicPlayerViewModel::next()
{
    if (m_songs.isEmpty()) return;
    int nextIdx;
    if (m_shuffleMode) {
        nextIdx = nextShuffleIndex();
    } else {
        nextIdx = (m_currentIndex + 1) % static_cast<int>(m_songs.count());
    }
    setCurrentIndex(nextIdx);
}

void MusicPlayerViewModel::prev()
{
    if (m_songs.isEmpty()) return;
    int prevIdx = (m_currentIndex - 1 < 0) ? static_cast<int>(m_songs.count()) - 1 : m_currentIndex - 1;
    setCurrentIndex(prevIdx);
}

void MusicPlayerViewModel::scanLibrary()
{
    if (m_isScanning) return;

    beginResetModel();
    m_songs.clear();
    m_currentIndex = -1;
    endResetModel();
    emit currentIndexChanged();

    m_isScanning = true;
    emit isScanningChanged();

    QString path = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    emit requestScan(path);
}

void MusicPlayerViewModel::onSongFound(const SongData& song)
{
    beginInsertRows(QModelIndex(), static_cast<int>(m_songs.count()), static_cast<int>(m_songs.count()));
    m_songs.append(song);
    endInsertRows();
}

void MusicPlayerViewModel::onScanFinished()
{
    m_isScanning = false;
    emit isScanningChanged();

    // Restore resume point (set source only, do NOT auto-play)
    if (m_lastIndex >= 0 && m_lastIndex < m_songs.count()) {
        m_currentIndex = m_lastIndex;
        emit currentIndexChanged();
        m_progress = 0.0f;
        emit progressChanged();
        if (m_player)
            m_player->setSource(QUrl::fromLocalFile(m_songs[m_currentIndex].filePath));
        // Mark resume pending so we can seek once duration is known
        if (m_lastPos > 0) {
            m_resumePending = true;
        }
        updatePlaybackState();
    }
}

void MusicPlayerViewModel::onPositionChanged(qint64 position)
{
    m_positionMs = position;
    emit positionChanged();

    if (m_player && m_player->duration() > 0) {
        m_progress = static_cast<float>(position) / static_cast<float>(m_player->duration());
        emit progressChanged();
    }

    scheduleResumeSave();
}

void MusicPlayerViewModel::onDurationChanged(qint64 duration)
{
    m_duration = duration;
    emit durationChanged();

    // Apply pending resume seek once we know the duration
    if (m_resumePending && duration > 0 && m_player) {
        m_resumePending = false;
        qint64 clamped = std::clamp(m_lastPos, static_cast<qint64>(0), duration);
        m_player->setPosition(clamped);
    }

    if (duration > 0 && m_player) {
        m_progress = static_cast<float>(m_player->position()) / static_cast<float>(duration);
        emit progressChanged();
    }
}

// ==== New internal slots (Step 1) ====
void MusicPlayerViewModel::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (!m_player)
        return;

    bool loading = (status == QMediaPlayer::LoadingMedia
                    || status == QMediaPlayer::BufferingMedia
                    || status == QMediaPlayer::StalledMedia);
    if (loading != m_isLoading) {
        m_isLoading = loading;
        emit isLoadingChanged();
    }

    if (status == QMediaPlayer::EndOfMedia) {
        switch (m_repeatMode) {
            case MusicEnums::RepeatMode::One:
                if (m_currentIndex >= 0) {
                    m_player->setPosition(0);
                    m_player->play();
                }
                break;
            case MusicEnums::RepeatMode::All:
                next();
                break;
            case MusicEnums::RepeatMode::Off:
            default:
                if (m_currentIndex >= 0 && m_currentIndex < m_songs.count() - 1) {
                    next();
                } else {
                    m_player->pause();
                }
                break;
        }
    }

    updatePlaybackState();
}

void MusicPlayerViewModel::onErrorOccurred(QMediaPlayer::Error /*error*/, const QString &errorString)
{
    QString msg = "Playback error: " + errorString;
    m_lastError = msg;
    emit lastErrorChanged();
    emit playbackError(msg);
}

void MusicPlayerViewModel::onPeriodicSaveTimeout()
{
    saveResumeNow();
}

// ==== New private helpers (Step 1) ====
void MusicPlayerViewModel::updatePlaybackState()
{
    MusicEnums::PlaybackState newState;
    if (m_isLoading) {
        newState = MusicEnums::PlaybackState::Loading;
    } else if (m_player && m_player->isPlaying()) {
        newState = MusicEnums::PlaybackState::Playing;
    } else if (m_player && m_player->playbackState() == QMediaPlayer::PausedState) {
        newState = MusicEnums::PlaybackState::Paused;
    } else {
        newState = MusicEnums::PlaybackState::Stopped;
    }

    if (newState != m_playbackState) {
        m_playbackState = newState;
        emit playbackStateChanged();
    }
}

void MusicPlayerViewModel::saveResumeNow()
{
    m_settings.setValue("music/lastIndex", m_currentIndex);
    m_settings.setValue("music/lastPositionMs", m_player ? m_player->position() : m_positionMs);
    m_settings.sync();
    m_lastSavedPosition = m_player ? m_player->position() : m_positionMs;
}

void MusicPlayerViewModel::scheduleResumeSave()
{
    qint64 cur = m_player ? m_player->position() : m_positionMs;
    if (std::abs(cur - m_lastSavedPosition) >= kSavePositionDeltaMs) {
        m_saveTimer->start();
    }
}

int MusicPlayerViewModel::nextShuffleIndex() const
{
    if (m_songs.count() <= 1) return m_currentIndex;
    int candidate;
    do {
        candidate = QRandomGenerator::global()->bounded(static_cast<int>(m_songs.count()));
    } while (candidate == m_currentIndex);
    return candidate;
}
