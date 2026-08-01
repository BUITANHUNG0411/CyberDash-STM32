#include <QtTest>
#include <QCoreApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <QMediaMetaData>
#include "viewmodels/MusicPlayerViewModel.h"

using namespace MusicEnums;

class TestMusicPlayback : public QObject
{
    Q_OBJECT

public:
    TestMusicPlayback() {}
    ~TestMusicPlayback() {}

private slots:
    void initTestCase() {
        QVERIFY(m_settingsDirectory.isValid());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                           m_settingsDirectory.path());
    }

    void disabledPersistenceDoesNotWriteSettings_test() {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                           QStringLiteral("QtStmAutomotiveSimulator"),
                           QStringLiteral("QtStmAutomotiveSimulator"));
        settings.setValue(QStringLiteral("music/lastIndex"), 42);
        settings.setValue(QStringLiteral("music/lastPositionMs"), 12345);
        settings.sync();

        {
            MusicPlayerViewModel vm(nullptr, false, false);
            vm.saveResume();
        }

        settings.sync();
        QCOMPARE(settings.value(QStringLiteral("music/lastIndex")).toInt(), 42);
        QCOMPARE(settings.value(QStringLiteral("music/lastPositionMs")).toLongLong(),
                 12345);
    }

    void cycleRepeat_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        QSignalSpy repeatSpy(&vm, &MusicPlayerViewModel::repeatModeChanged);

        // Off -> One -> All -> Off
        QCOMPARE(vm.repeatMode(), RepeatMode::Off);
        vm.cycleRepeat();
        QCOMPARE(vm.repeatMode(), RepeatMode::One);
        vm.cycleRepeat();
        QCOMPARE(vm.repeatMode(), RepeatMode::All);
        vm.cycleRepeat();
        QCOMPARE(vm.repeatMode(), RepeatMode::Off);

        QCOMPARE(repeatSpy.count(), 3);
    }

    void toggleShuffle_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        QSignalSpy shuffleSpy(&vm, &MusicPlayerViewModel::shuffleModeChanged);

        QCOMPARE(vm.shuffleMode(), false);
        vm.toggleShuffle();
        QCOMPARE(vm.shuffleMode(), true);
        vm.toggleShuffle();
        QCOMPARE(vm.shuffleMode(), false);

        QCOMPARE(shuffleSpy.count(), 2);
    }

    void volume_clamp_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        QSignalSpy volSpy(&vm, &MusicPlayerViewModel::volumeChanged);

        vm.setVolume(-0.5f);
        QCOMPARE(vm.volume(), 0.0f);

        vm.setVolume(2.0f);
        QCOMPARE(vm.volume(), 1.0f);

        // In-range value applied without clamping
        vm.setVolume(0.5f);
        QCOMPARE(vm.volume(), 0.5f);

        // At least the two out-of-range sets should have emitted
        QCOMPARE(volSpy.count(), 3);
    }

    void volume_from_position_clamps_test() {
        MusicPlayerViewModel vm(nullptr, false, false);

        vm.setVolumeFromPosition(-10.0, 100.0);
        QCOMPARE(vm.volume(), 0.0f);

        vm.setVolumeFromPosition(150.0, 100.0);
        QCOMPARE(vm.volume(), 1.0f);
    }

    void volume_from_position_normalizes_test() {
        MusicPlayerViewModel vm(nullptr, false, false);

        vm.setVolumeFromPosition(25.0, 100.0);
        QCOMPARE(vm.volume(), 0.25f);
    }

    void volume_from_position_ignores_nonpositive_width_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        vm.setVolume(0.4f);

        vm.setVolumeFromPosition(20.0, 0.0);
        QCOMPARE(vm.volume(), 0.4f);

        vm.setVolumeFromPosition(20.0, -10.0);
        QCOMPARE(vm.volume(), 0.4f);
    }

    void seek_ratio_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        // No media loaded -> duration() == 0 -> seek is a no-op (must not crash).
        vm.seek(0.5f);
        vm.seekMs(1000);
        // Pure clamping logic for ratio: clamp helper behaviour is covered by
        // volume_clamp_test; seek guards duration<=0 so this only exercises safety.
        QVERIFY(true);
    }

    void playbackState_reporting_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        // Default state is Stopped (no loading, not playing).
        QCOMPARE(vm.playbackState(), PlaybackState::Stopped);
        // volume default 1.0, shuffle off, repeat off validated indirectly here
        QCOMPARE(vm.volume(), 1.0f);
        QCOMPARE(vm.shuffleMode(), false);
        QCOMPARE(vm.repeatMode(), RepeatMode::Off);
    }

    void scrubber_clamps_and_tracks_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        QSignalSpy spy(&vm, &MusicPlayerViewModel::scrubberStateChanged);

        vm.beginScrub(150.0, 100.0);
        QCOMPARE(vm.scrubberDragging(), true);
        QCOMPARE(vm.scrubberRatio(), 1.0f);

        vm.updateScrub(-10.0, 100.0);
        QCOMPARE(vm.scrubberRatio(), 0.0f);

        vm.endScrub();
        QCOMPARE(vm.scrubberDragging(), false);
        QVERIFY(spy.size() >= 3);
    }

    void scrubber_ignores_zero_width_test() {
        MusicPlayerViewModel vm(nullptr, false, false);

        vm.beginScrub(20.0, 0.0);
        QCOMPARE(vm.scrubberDragging(), true);
        QCOMPARE(vm.scrubberRatio(), 0.0f);
    }

    void first_scanned_song_becomes_current_test() {
        MusicPlayerViewModel vm(nullptr, false, false);
        SongData song;
        song.title = QStringLiteral("Qt FLAC Title");
        song.artist = QStringLiteral("Qt FLAC Artist");
        song.filePath = QStringLiteral("/tmp/qt-flac-title.flac");

        QCOMPARE(vm.currentIndex(), -1);
        QVERIFY(QMetaObject::invokeMethod(&vm, "onSongFound", Qt::DirectConnection,
                                          Q_ARG(SongData, song)));
        QCOMPARE(vm.rowCount(), 1);
        QCOMPARE(vm.currentIndex(), 0);
        QCOMPARE(vm.currentTitle(), QStringLiteral("Qt FLAC Title"));
        QCOMPARE(vm.currentArtist(), QStringLiteral("Qt FLAC Artist"));
        QCOMPARE(vm.data(vm.index(0, 0), MusicPlayerViewModel::TitleRole).toString(),
                 QStringLiteral("Qt FLAC Title"));
    }

    void qt_metadata_overrides_filename_fallback_test() {
        SongData song;
        song.title = QStringLiteral("file name");
        song.artist = QStringLiteral("Unknown Artist");
        QMediaMetaData metadata;
        metadata.insert(QMediaMetaData::Title, QStringLiteral("Tag title"));
        metadata.insert(QMediaMetaData::ContributingArtist, QStringLiteral("Tag artist"));
        metadata.insert(QMediaMetaData::AlbumTitle, QStringLiteral("Tag album"));

        MusicScanner::applyQtMetaData(song, metadata);

        QCOMPARE(song.title, QStringLiteral("Tag title"));
        QCOMPARE(song.artist, QStringLiteral("Tag artist"));
        QCOMPARE(song.album, QStringLiteral("Tag album"));
    }

private:
    QTemporaryDir m_settingsDirectory;
};

QTEST_MAIN(TestMusicPlayback)

#include "tst_music_playback.moc"
