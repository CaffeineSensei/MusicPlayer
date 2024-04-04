#ifndef WORKER_H
#define WORKER_H

#include "song.h"
#include <QFileInfo>
#include <QDebug>
#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker();
signals:
    void getASongFailed();
    void getASongFinished();    //解析结束信号
public slots:
    void getASong(const QUrl & mp3Url); //解析歌曲，参数接收歌曲路径
    void readLyrics(Song * song);       //解析歌词，参数接收歌曲对象
    Song* sendSong();
private:
    Song *song;
};

#endif // WORKER_H
