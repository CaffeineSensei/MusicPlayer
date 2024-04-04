#include "dialogshowlyrics.h"
#include "ui_dialogshowlyrics.h"

DialogShowLyrics::DialogShowLyrics(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogShowLyrics)
{
    ui->setupUi(this);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

DialogShowLyrics::~DialogShowLyrics()
{
    delete ui;
}

void DialogShowLyrics::getLyricsFromWidget(Song * song)
{
    lyrics = song->lyrics();
    //ui->listWidget->addItem(new QListWidgetItem(song->name() + " - " + song->artist()));
}

void DialogShowLyrics::getMediaPlayerPosition(qint64 pos)
{
    position = pos;
}

void DialogShowLyrics::loadLyrics()
{
    ui->listWidget->clear();
    if (lyrics.isEmpty())
    {
        QListWidgetItem * item = new QListWidgetItem("无歌词");
        item->setTextAlignment(Qt::AlignCenter);    //设置该行文本居中显示
        ui->listWidget->addItem(item);
        current.clear();
        return;
    }

    qDebug() << "更新所有歌词，第一行文本：" << lyrics.first();
    for (auto text : lyrics) //获取lyrcis中的每行歌词的文本到text
    {
        //LOG << "text: " << text;
        QListWidgetItem * item = new QListWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);    //设置该行文本居中显示
        ui->listWidget->addItem(item);
    }
}

void DialogShowLyrics::showLyrics()
{
    if (lyrics.isEmpty())
    {
        //LOG << "当前歌曲没有歌词，不用同步";
        return;
    }

    //LOG << "实时刷新当前歌词，播放器当前进度：" << m_mediaPlayer->position();
    int index = 0;  //存储当前进度对应的歌词文本的索引

    //循环遍历歌词容器，从头往后找
    for (auto it = lyrics.begin(); it != lyrics.end(); it++)
    {
        //end前面的最后一个元素
        if (lyrics.end() == (it + 1)) { break; }

        //比较播放进度和时间戳，播放进度大于等于某一行（的起始事件），并且小于下一行
        if (position >= it.key() && position < (it + 1).key())
        {
            // qDebug() << "当前歌词时间范围和播放器进度及文本：["
            //     << it.key() << ", " << (it + 1).key() << ") "
            //     << position << " "
            //     << it.value();
            current = it.value();
            break;
        }

        index++;    //如果不是这一行，就继续判断下一行
    }

    ui->listWidget->setCurrentRow(index);    //界面歌词控件设置当前行，高亮显示
    QListWidgetItem * item = ui->listWidget->item(index);    //获取当前行元素
    ui->listWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter); //列表滚动到该行，并垂直居中
}

void DialogShowLyrics::stopShow()
{
    ui->listWidget->clear();
    current.clear();
}
