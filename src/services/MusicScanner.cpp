#include "MusicScanner.h"
#include <QDirIterator>
#include <QEventLoop>
#include <QFileInfo>
#include <QThread>
#include <QHash>
#include <QByteArray>
#include <QBuffer>
#include <QImage>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QTimer>
#include <QUrl>

MusicScanner::MusicScanner(QObject* parent) : QObject(parent) {}

void MusicScanner::applyQtMetaData(SongData& song, const QMediaMetaData& metadata)
{
    const QString title = metadata.stringValue(QMediaMetaData::Title).trimmed();
    if (!title.isEmpty())
        song.title = title;

    QString artist = metadata.stringValue(QMediaMetaData::ContributingArtist).trimmed();
    if (artist.isEmpty())
        artist = metadata.stringValue(QMediaMetaData::AlbumArtist).trimmed();
    if (!artist.isEmpty())
        song.artist = artist;

    const QString album = metadata.stringValue(QMediaMetaData::AlbumTitle).trimmed();
    if (!album.isEmpty())
        song.album = album;

    QImage coverArt = metadata.value(QMediaMetaData::CoverArtImage).value<QImage>();
    if (coverArt.isNull())
        coverArt = metadata.value(QMediaMetaData::ThumbnailImage).value<QImage>();

    if (!coverArt.isNull()) {
        QByteArray encodedCover;
        QBuffer buffer(&encodedCover);
        if (buffer.open(QIODevice::WriteOnly) && coverArt.save(&buffer, "PNG"))
            song.coverArt = "data:image/png;base64," + QString::fromLatin1(encodedCover.toBase64());
    }
}

void MusicScanner::scanLibrary(const QString& path) {
    QDirIterator it(path, QStringList() << "*.mp3" << "*.flac" << "*.wav", QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext() && !QThread::currentThread()->isInterruptionRequested()) {
        QString filePath = it.next();
        QFileInfo fi(filePath);
        
        SongData song;
        song.filePath = filePath;
        
        // Mock metadata extraction from filename to avoid heavy dependencies
        QString baseName = fi.completeBaseName();
        QStringList parts = baseName.split("-");
        if (parts.size() >= 2) {
            song.artist = parts[0].trimmed();
            song.title = parts[1].trimmed();
        } else {
            song.artist = "Unknown Artist";
            song.title = baseName;
        }

        // Generate consistent mock colors based on filename hash to maintain Neon aesthetic
        const uint hash = static_cast<uint>(qHash(baseName));
        song.color1 = QString("#%1").arg(hash & 0xFFFFFF, 6, 16, QChar('0'));
        song.color2 = QString("#%1").arg((hash >> 8) & 0xFFFFFF, 6, 16, QChar('0'));

        QMediaPlayer metadataPlayer;
        QEventLoop metadataLoop;
        QTimer timeout;
        bool probeComplete = false;
        timeout.setSingleShot(true);

        const auto finishProbe = [&metadataLoop, &probeComplete]() {
            probeComplete = true;
            if (metadataLoop.isRunning())
                metadataLoop.quit();
        };
        connect(&metadataPlayer, &QMediaPlayer::metaDataChanged, &metadataLoop, finishProbe);
        connect(&metadataPlayer, &QMediaPlayer::mediaStatusChanged, &metadataLoop,
                [&finishProbe](QMediaPlayer::MediaStatus status) {
                    if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::InvalidMedia)
                        finishProbe();
                });
        connect(&timeout, &QTimer::timeout, &metadataLoop, &QEventLoop::quit);

        metadataPlayer.setSource(QUrl::fromLocalFile(filePath));
        if (!probeComplete) {
            timeout.start(1500);
            metadataLoop.exec();
        }
        applyQtMetaData(song, metadataPlayer.metaData());

        emit songFound(song);
        QThread::msleep(10); // Artificial slight delay to prevent UI flooding
    }
    emit scanFinished();
}
