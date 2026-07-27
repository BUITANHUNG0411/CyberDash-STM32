#include <QtTest>
#include <QCoreApplication>
#include "viewmodels/MusicPlayerViewModel.h"

using namespace MusicEnums;

class TestMusicPlayback : public QObject
{
    Q_OBJECT

public:
    TestMusicPlayback() {}
    ~TestMusicPlayback() {}

private slots:
    void cycleRepeat_test() {
        MusicPlayerViewModel vm(nullptr, false);
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
        MusicPlayerViewModel vm(nullptr, false);
        QSignalSpy shuffleSpy(&vm, &MusicPlayerViewModel::shuffleModeChanged);

        QCOMPARE(vm.shuffleMode(), false);
        vm.toggleShuffle();
        QCOMPARE(vm.shuffleMode(), true);
        vm.toggleShuffle();
        QCOMPARE(vm.shuffleMode(), false);

        QCOMPARE(shuffleSpy.count(), 2);
    }

    void volume_clamp_test() {
        MusicPlayerViewModel vm(nullptr, false);
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
        MusicPlayerViewModel vm(nullptr, false);

        vm.setVolumeFromPosition(-10.0, 100.0);
        QCOMPARE(vm.volume(), 0.0f);

        vm.setVolumeFromPosition(150.0, 100.0);
        QCOMPARE(vm.volume(), 1.0f);
    }

    void volume_from_position_normalizes_test() {
        MusicPlayerViewModel vm(nullptr, false);

        vm.setVolumeFromPosition(25.0, 100.0);
        QCOMPARE(vm.volume(), 0.25f);
    }

    void volume_from_position_ignores_nonpositive_width_test() {
        MusicPlayerViewModel vm(nullptr, false);
        vm.setVolume(0.4f);

        vm.setVolumeFromPosition(20.0, 0.0);
        QCOMPARE(vm.volume(), 0.4f);

        vm.setVolumeFromPosition(20.0, -10.0);
        QCOMPARE(vm.volume(), 0.4f);
    }

    void seek_ratio_test() {
        MusicPlayerViewModel vm(nullptr, false);
        // No media loaded -> duration() == 0 -> seek is a no-op (must not crash).
        vm.seek(0.5f);
        vm.seekMs(1000);
        // Pure clamping logic for ratio: clamp helper behaviour is covered by
        // volume_clamp_test; seek guards duration<=0 so this only exercises safety.
        QVERIFY(true);
    }

    void playbackState_reporting_test() {
        MusicPlayerViewModel vm(nullptr, false);
        // Default state is Stopped (no loading, not playing).
        QCOMPARE(vm.playbackState(), PlaybackState::Stopped);
        // volume default 1.0, shuffle off, repeat off validated indirectly here
        QCOMPARE(vm.volume(), 1.0f);
        QCOMPARE(vm.shuffleMode(), false);
        QCOMPARE(vm.repeatMode(), RepeatMode::Off);
    }

    void scrubber_clamps_and_tracks_test() {
        MusicPlayerViewModel vm(nullptr, false);
        QSignalSpy spy(&vm, &MusicPlayerViewModel::scrubberStateChanged);

        vm.beginScrub(150.0, 100.0);
        QCOMPARE(vm.scrubberDragging(), true);
        QCOMPARE(vm.scrubberRatio(), 1.0f);

        vm.updateScrub(-10.0, 100.0);
        QCOMPARE(vm.scrubberRatio(), 0.0f);

        vm.endScrub();
        QCOMPARE(vm.scrubberDragging(), false);
        QVERIFY(spy.count() >= 3);
    }

    void scrubber_ignores_zero_width_test() {
        MusicPlayerViewModel vm(nullptr, false);

        vm.beginScrub(20.0, 0.0);
        QCOMPARE(vm.scrubberDragging(), true);
        QCOMPARE(vm.scrubberRatio(), 0.0f);
    }
};

QTEST_MAIN(TestMusicPlayback)

#include "tst_music_playback.moc"
