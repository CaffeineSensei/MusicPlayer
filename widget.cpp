#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QTimer>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_mediaPlayer(new QMediaPlayer)
    , m_audioOutput(new QAudioOutput)
    , database(DatabaseManager::getInstance())
    , musicList(new DialogMusicList(m_mediaPlayer,database,this))
    , showLyrics(new DialogShowLyrics(this))
{
    ui->setupUi(this);
    m_thread = new QThread(this);
    wo = new Worker();
    initWorker();
    initTimer();
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    if(database->createConnect())
    {
        database->initDatabase();
        loadLastPlayed();
        musicList->loadList();
    }
    connect(m_mediaPlayer,&QMediaPlayer::mediaStatusChanged,this,&Widget::onMediaStatusChanged);//媒体播放器状态改变时触发
    connect(m_mediaPlayer,&QMediaPlayer::playingChanged,this,&Widget::onMediaPlayingChanged);//音乐播放状态改变时触发
    connect(m_mediaPlayer,&QMediaPlayer::positionChanged,this,&Widget::onPositionChanged);   //音乐播放进度改变时触发
    connect(musicList,&QDialog::finished,this,&Widget::onMusicListWindowClosed); //播放列表窗口关闭时触发
    connect(showLyrics,&QDialog::finished,this,&Widget::onShowLyricWindowClosed);//歌词显示窗口关闭时触发
    //connect(this, SIGNAL(sigSpectrumChanged(vector<float>)), m_spectrograph, SLOT(handleSpectrumChanged(vector<float>)));

}

Widget::~Widget()
{
    if (m_thread && m_thread->isRunning())  //空指针判断，和正在运行判断
    {
        m_thread->quit();       //退出
        m_thread->wait(5000);   //等它退出完，最多等五秒
    }

    delete m_audioOutput;
    delete m_mediaPlayer;
    delete musicList;
    delete showLyrics;
    delete ui;
}

//定时器，每100ms一次
void Widget::initTimer()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(100);  //设置超时间隔100毫秒
    m_timer->start();
}

void Widget::loadLastPlayed()
{
    QSqlQuery query;
    position = 0;
    query.prepare("select * from tb_playlist where current = 1");
    if(query.exec())
    {
        if(query.first())
        {
            QUrl url;
            QString fileName;
            url = QUrl(query.value("url").toString());
            fileName = query.value("name").toString();
            position = query.value("position").toInt();
            qDebug() << position;
            ui->label_song->setText(fileName);
            m_mediaPlayer->setSource(url);
        }

    }
    else
    {
        qDebug() << "读取上次播放内容失败";
    }
}

//初始化一个工作台，主要处理歌曲信息解析
void Widget::initWorker()
{
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater); //线程结束后，延后回收
    connect(m_thread, &QThread::finished, wo, &Worker::deleteLater);
    connect(wo, &Worker::getASongFinished, this, &Widget::handle_worker_getASongFinished); //歌曲信息解析完成时连接的槽函数
    connect(wo, &Worker::getASongFailed,this,&Widget::handle_worker_getASongFailed);//歌曲信息解析失败（无歌词）时连接的槽函数
    wo->moveToThread(m_thread);
    m_thread->start();                //线程启动
}

//窗口拖动事件，所有已显示的窗口都跟随移动
void Widget::moveEvent(QMoveEvent *event)
{
    musicList->move(this->x(),this->y()+this->height()+30);//播放列表窗口在主窗口下边缘对齐出现
    showLyrics->move(this->x()+this->width(),this->y());   //歌词显示列表在主窗口有边缘对齐出现
    QPoint newPos = musicList->pos() + event->pos() - event->oldPos();
    musicList->move(newPos);
    newPos = showLyrics->pos() + event->pos() - event->oldPos();
    showLyrics->move(newPos);
}

void Widget::closeEvent(QCloseEvent *event)
{
    database->deleteData();
    musicList->record();
    database->destoryConnect();
    event->accept();
}

void Widget::wheelEvent(QWheelEvent *event)
{
    int delta = event->angleDelta().y();
    float volume = m_audioOutput->volume();
    if (delta > 0) {
        // 向上滚动，增加音量
        volume += 0.05; // 可以根据需要调整增量
        if (volume > 1) volume = 1.0; // 确保音量不超过最大值
    }
    else
    {
        // 向下滚动，减少音量
        volume -= 0.05; // 可以根据需要调整减量
        if (volume < 0) volume = 0; // 确保音量不低于最小值
    }
    m_audioOutput->setVolume(volume);
}

//播放状态改变时改变播放、暂停按钮样式
void Widget::onMediaPlayingChanged()
{
    if(m_mediaPlayer->isPlaying())
    {
        ui->pushButton_playOrpause->setStyleSheet("image: url(:/musicPlayer/icon/icon/pause.svg);");
    }
    else
    {
        ui->pushButton_playOrpause->setStyleSheet("image: url(:/musicPlayer/icon/icon/play.svg);");
    }
}

//媒体播放器状态改变槽函数，“加载完成”时，同时加载歌曲信息；“播放结束”时，若已启用单曲循环，则执行单曲循环，否则
void Widget::onMediaStatusChanged(QMediaPlayer::MediaStatus state)
{
    if(state == QMediaPlayer::MediaStatus::LoadedMedia)
    {
        m_mediaPlayer->setPosition(position);
        QUrl currentMusic = m_mediaPlayer->source();
        ui->label_song->setText(currentMusic.fileName());
        wo->getASong(currentMusic);
        position = 0;
    }
    if(state == QMediaPlayer::MediaStatus::EndOfMedia)
    {
        if(oneLoop)
        {
            musicList->playOneLoop();
        }
        else
        {
            //disconnect(m_timer, &QTimer::timeout, this, &Widget::handle_timer_timeout);
            musicList->nextSong();
        }
    }
}

//音乐位置变化时，进度条跟随变化
void Widget::onPositionChanged(qint64 position)
{
    qint64 seconds = position / 1000; //秒级的当前播放时间进度
    qint64 durationSeconds = m_mediaPlayer->duration() / 1000; //秒级歌曲时长

    int sliderMax = ui->progressBar->maximum(); //进度条最大值
    //前端进度条的值 = 后台当前播放进度 / 后台歌曲时长 * 前端进度条最大值
    ui->progressBar->setValue( (double)seconds / durationSeconds * sliderMax );
    //使用QString占位符处理时间标签文本（补0）: QString("%1").arg(进度数值, 显示位数2, 进制10, 填充字符QChar('0'));
    QString text = QString("%1:%2/%3:%4").arg(seconds / 60, 2, 10, QChar('0')) //当前分钟数
                       .arg(seconds % 60, 2, 10, QChar('0')) //当前秒数
                       .arg(durationSeconds / 60, 2, 10, QChar('0')) //时长分钟数
                       .arg(durationSeconds % 60, 2, 10, QChar('0')); //时长秒数
    ui->label_position->setText(text);
}

//播放列表窗口关闭时，更改按钮样式
void Widget::onMusicListWindowClosed()
{
    ui->pushButton_list->setStyleSheet("image: url(:/musicPlayer/icon/icon/playlistMusic4.svg);");
}

//播放列表显示开关按钮
void Widget::on_pushButton_list_clicked()
{
    if(musicList->isVisible())
    {
        musicList->close();
    }
    else
    {
        ui->pushButton_list->setStyleSheet("image: url(:/musicPlayer/icon/icon/playlistMusic3.svg);");
        musicList->move(this->x(),this->y()+this->height()+30);
        musicList->show();
        musicList->exec();
    }
}

//播放、暂停切换按钮
void Widget::on_pushButton_playOrpause_clicked()
{
    if (m_mediaPlayer->isPlaying())
    {
        m_mediaPlayer->pause();
    }
    else
    {
        m_mediaPlayer->play();
    }
}

//拖拽进度条时，跳转到对应的音乐播放位置，并将解除的进度条随音乐位置变化的连接恢复
void Widget::on_progressBar_sliderReleased()
{
    int sliderValue = ui->progressBar->value();
    int sliderMaxValue = ui->progressBar->maximum();
    //log << sliderValue << " " << sliderMaxValue;
    //后台播放进度 = 前端进度条 / 进度条最大值 * 后台歌曲时长
    m_mediaPlayer->setPosition(((double)sliderValue  * m_mediaPlayer->duration())/ sliderMaxValue);
    connect(m_mediaPlayer,&QMediaPlayer::positionChanged,this,&Widget::onPositionChanged); //恢复连接
}

//拖拽进度条时，会与音乐播放进度改变信号冲突，需要暂时解除连接
void Widget::on_progressBar_sliderPressed()
{
    disconnect(m_mediaPlayer,&QMediaPlayer::positionChanged,this,&Widget::onPositionChanged);
}

//上一曲
void Widget::on_pushButton_last_clicked()
{
    musicList->previousSong();
}

//下一曲
void Widget::on_pushButton_next_clicked()
{
    musicList->nextSong();
}

//单曲循环、列表循环切换按钮，修改对应的样式
void Widget::on_pushButton_loop_clicked()
{
    if(oneLoop)
    {
        oneLoop = false;
        ui->pushButton_loop->setStyleSheet("image: url(:/musicPlayer/icon/icon/allloop.svg);");
    }
    else
    {
        oneLoop = true;
        ui->pushButton_loop->setStyleSheet("image: url(:/musicPlayer/icon/icon/oneloop.svg);");
    }
}

//“停止按钮”按下时，停止播放音乐
void Widget::on_pushButton_stop_clicked()
{
    m_mediaPlayer->stop();
}

//定时器时间溢出处理函数（每100ms一次），获取歌曲播放位置，刷新歌词显示；若已启用了标题歌词，则从歌词显示页面获取当前一句歌词并在标题显示
void Widget::handle_timer_timeout()
{
    showLyrics->getMediaPlayerPosition(m_mediaPlayer->position());
    showLyrics->showLyrics();
    if(titleLyricsEnable)
    {
        QString titleLyrics = showLyrics->sendCurrentLyrics();
        this->setWindowTitle(titleLyrics);
    }
    else
    {
        this->setWindowTitle("");
    }
}

//获取歌曲信息完成时的处理函数，worker传递“歌曲”，showLyrics获取“歌曲”，加载歌词，连接定时器溢出处理函数，进行歌词实时显示
void Widget::handle_worker_getASongFinished()
{
    Song * song = wo->sendSong();
    if (song != nullptr) //不为空则处理
    {
        qDebug() << "读取到歌曲, 存储到歌曲管理员（负责释放）：" << *song;
        showLyrics->getLyricsFromWidget(song);
        showLyrics->loadLyrics();
        connect(m_timer, &QTimer::timeout, this, &Widget::handle_timer_timeout);
    }
}

//获取歌曲信息失败时，解除定时器时间溢出处理函数的连接，同时清空歌词显示
void Widget::handle_worker_getASongFailed()
{
    disconnect(m_timer, &QTimer::timeout, this, &Widget::handle_timer_timeout);
    showLyrics->stopShow();
    this->setWindowTitle("");
}

//歌词显示窗口关闭时，更改“歌词按钮”样式，启用标题歌词显示
void Widget::onShowLyricWindowClosed()
{
    ui->pushButton_lyric->setStyleSheet("image: url(:/musicPlayer/icon/icon/geciweidianji.svg);");
    titleLyricsEnable = true;
}

//点击“歌词按钮”时，若窗口为关闭状态，将其显示，并更改按钮样式，禁用标题歌词显示；若窗口已显示，将其关闭
void Widget::on_pushButton_lyric_clicked()
{
    if(showLyrics->isVisible())
    {
        showLyrics->close();
    }
    else
    {
        ui->pushButton_lyric->setStyleSheet("image: url(:/musicPlayer/icon/icon/gecidianji.svg);");
        titleLyricsEnable = false;
        showLyrics->move(this->x()+this->width(),this->y());
        showLyrics->show();
        showLyrics->exec();
    }
}
