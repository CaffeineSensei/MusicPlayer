#ifndef WIDGET_H
#define WIDGET_H
#include "dialogmusiclist.h"
#include "dialogshowlyrics.h"
#include "databasemanager.h"
#include "worker.h"
#include <QWidget>
#include <QAudioOutput>


QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
public:
    void initWorker();
    void initTimer();
    void loadLastPlayed();
protected:
    void moveEvent(QMoveEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    QThread * m_thread; //子线程，工作线程，处理界面交互以外的工作
    Worker * wo;
    Song * song;
signals:
    void addSong(const QUrl& mp3Url); //添加歌曲信号
private slots:
    void onMediaPlayingChanged();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus state);
    void onPositionChanged(qint64 position);
    void onMusicListWindowClosed();
    void onShowLyricWindowClosed();
    void on_pushButton_list_clicked();
    void on_pushButton_playOrpause_clicked();
    void on_progressBar_sliderReleased();
    void on_progressBar_sliderPressed();
    void on_pushButton_last_clicked();
    void on_pushButton_next_clicked();
    void on_pushButton_loop_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_lyric_clicked();

    void handle_worker_getASongFailed();
    void handle_worker_getASongFinished();
    void handle_timer_timeout();

private:
    bool oneLoop = false;
    bool titleLyricsEnable = true;
    int position;
    Ui::Widget *ui;
    QTimer * m_timer;
    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;
    DatabaseManager *database;
    DialogMusicList *musicList;
    DialogShowLyrics *showLyrics;

};
#endif // WIDGET_H
