#ifndef DIALOGMUSICLIST_H
#define DIALOGMUSICLIST_H

#include <QMediaPlayer>
#include <QDialog>
#include <QDir>
#include <QStringList>
#include <QFileDialog>
#include <QtMultimedia>
#include <QListWidget>
#include "databasemanager.h"

namespace Ui {
class DialogMusicList;
}

class DialogMusicList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMusicList(QMediaPlayer *mediaPlayer = nullptr,DatabaseManager *database = nullptr ,QWidget *parent = nullptr);
    ~DialogMusicList();
public:
    QUrl getUrlFromItem(QListWidgetItem *item);
    void previousSong();
    void nextSong();
    void playOneLoop();
    void record();
    void loadList();
private slots:
    void on_pushButton_add_clicked();
    void on_listWidget_doubleClicked(const QModelIndex &index);

    void on_pushButton_remove_clicked();

    void on_pushButton_clear_clicked();

private:
    Ui::DialogMusicList *ui;
    QMediaPlayer *mediaPlayer;
    DatabaseManager *playListData;
};

#endif // DIALOGMUSICLIST_H
