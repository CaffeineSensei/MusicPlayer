#ifndef SONG_H
#define SONG_H

#include <QUrl>
#include <QString>
#include <QMap>
class Song
{
private:
    QUrl m_url;
    QString m_name;
    QString m_artist;
    QString m_album;
    QMap<qint64, QString> m_lyrics; //键值对容器，存储歌词时间戳（毫秒级播放进度），和歌词文本
public:
    Song();
    Song(const QUrl & url,
         const QString & name,
         const QString & artist,
         const QString & album)
        : m_url(url), m_name(name), m_artist(artist), m_album(album){}
    const QUrl & url() const { return m_url; }
    QUrl url() { return m_url; }
    //set方法
    void url(const QUrl & url) { m_url = url; }

    const QString & name() const { return m_name; }
    const QString & artist() const { return m_artist; }
    const QString & album() const { return m_album; }
    void name(const QString & name) { m_name = name; }
    void artist(const QString & artist) { m_artist = artist; }
    void album(const QString & album) { m_album = album; }

    const QMap<qint64, QString> & lyrics() const        { return m_lyrics; }
    void lyrics(const QMap<qint64, QString> & lyrics)   { m_lyrics = lyrics; }
    friend QDebug& operator<<(QDebug & debug, const Song & song);
};

#endif // SONG_H
