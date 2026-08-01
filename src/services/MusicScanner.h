#pragma once

#include <QObject>
#include <QString>

class QMediaMetaData;

struct SongData {
    QString title;
    QString artist;
    QString album;
    QString filePath;
    QString color1;
    QString color2;
    QString coverArt;
};

Q_DECLARE_METATYPE(SongData)

class MusicScanner : public QObject
{
    Q_OBJECT
public:
    explicit MusicScanner(QObject* parent = nullptr);

    static void applyQtMetaData(SongData& song, const QMediaMetaData& metadata);

public slots:
    void scanLibrary(const QString& path);

signals:
    void songFound(const SongData& song);
    void scanFinished();
};
