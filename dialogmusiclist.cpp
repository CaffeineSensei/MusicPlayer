#include "dialogmusiclist.h"
#include "ui_dialogmusiclist.h"

DialogMusicList::DialogMusicList(QMediaPlayer *mediaPlayer,DatabaseManager *database,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMusicList)
    ,mediaPlayer(mediaPlayer)
    ,playListData(database)
{
    ui->setupUi(this);
    ui->listWidget->installEventFilter(this);
    ui->listWidget->setDragEnabled(true);
    ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);
}

QUrl DialogMusicList::getUrlFromItem(QListWidgetItem *item)
{
    QVariant itemData = item->data(Qt::UserRole);
    QUrl source = itemData.value<QUrl>();
    return source;
}

void DialogMusicList::previousSong()
{
    int count = ui->listWidget->count();
    int curRow = ui->listWidget->currentRow();
    curRow--;
    if(curRow < 0)
    {
        curRow = count - 1;
    }
    ui->listWidget->setCurrentRow(curRow);
    mediaPlayer->setSource(getUrlFromItem(ui->listWidget->currentItem()));
    mediaPlayer->play();
}

void DialogMusicList::nextSong()
{
    int count = ui->listWidget->count();
    int curRow = ui->listWidget->currentRow();
    if(count == 1)
    {
        mediaPlayer->play();
    }
    else
    {
        curRow++;
        curRow = curRow >= count ? 0:curRow;
        ui->listWidget->setCurrentRow(curRow);
        mediaPlayer->setSource(getUrlFromItem(ui->listWidget->currentItem()));
        mediaPlayer->play();
    }

}

void DialogMusicList::playOneLoop()
{
    mediaPlayer->play();
}

void DialogMusicList::record()
{
    int count = ui->listWidget->count();
    int current = ui->listWidget->currentRow();
    qDebug() <<"count:" << count;
    if(count > 0)
    {
        QUrl url;
        for (int i = 0; i < count; ++i)
        {
            url = getUrlFromItem(ui->listWidget->item(i));
            if(i == current)
            {
                playListData->recordCurrent(url,mediaPlayer->position());
            }
            else
            {
                playListData->recordList(url);
            }

            //qDebug() << url;
        }
    }
}

void DialogMusicList::loadList()
{
    QSqlQuery query;
    query.prepare("select * from tb_playlist");
    if(!query.exec())
    {
        qDebug() << "读取播放列表失败";
        return;
    }
    int i = 0;
    while (query.next())
    {
        QUrl url =QUrl(query.value("url").toString());
        QFileInfo fileInfo(url.toLocalFile());
        QListWidgetItem *aItem = new QListWidgetItem(fileInfo.fileName());
        aItem->setData(Qt::UserRole,QUrl::fromLocalFile(url.toLocalFile()));
        ui->listWidget->addItem(aItem);
        if(url == mediaPlayer->source())
        {
            ui->listWidget->setCurrentRow(i);
        }
        i++;
    }
}

DialogMusicList::~DialogMusicList()
{
    delete ui;
}

void DialogMusicList::on_pushButton_add_clicked()
{
    QString curPath = QDir::homePath();
    QString dlgTitle = "选择音频文件";
    QString filter = "音频文件(*.mp3 *.wav *.wma);;所有文件(*.*)";
    QStringList fileList = QFileDialog::getOpenFileNames(this,dlgTitle,curPath,filter);
    if(fileList.count() < 1)
    {
        return;
    }
    for (int i = 0; i < fileList.size(); ++i)
    {
        QString aFile = fileList.at(i);
        QFileInfo fileInfo(aFile);
        QListWidgetItem *aItem = new QListWidgetItem(fileInfo.fileName());
        aItem->setData(Qt::UserRole,QUrl::fromLocalFile(aFile));
        ui->listWidget->addItem(aItem);
    }
}

void DialogMusicList::on_listWidget_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    mediaPlayer->setSource(getUrlFromItem(ui->listWidget->currentItem()));
    mediaPlayer->play();

}

void DialogMusicList::on_pushButton_remove_clicked()
{
    int index = ui->listWidget->currentRow();
    if((mediaPlayer->isPlaying())&&(mediaPlayer->source() == getUrlFromItem(ui->listWidget->currentItem())))
    {
        nextSong();
    }
    if(index >= 0)
    {
        QListWidgetItem *item = ui->listWidget->takeItem(index);
        delete item;
    }
}


void DialogMusicList::on_pushButton_clear_clicked()
{
    ui->listWidget->clear();
    mediaPlayer->stop();
}

