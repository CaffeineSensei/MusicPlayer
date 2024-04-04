#ifndef DIALOGSHOWLYRICS_H
#define DIALOGSHOWLYRICS_H
#include "Song.h"
#include <QDialog>

namespace Ui {
class DialogShowLyrics;
}

class DialogShowLyrics : public QDialog
{
    Q_OBJECT

public:
    explicit DialogShowLyrics(QWidget *parent = nullptr);
    ~DialogShowLyrics();

public:
    void getLyricsFromWidget(Song * song);
    void getMediaPlayerPosition(qint64 pos);
    void showLyrics();
    void loadLyrics();
    void stopShow();
    const QString & sendCurrentLyrics() const { return current; }

private:
    Ui::DialogShowLyrics *ui;
    QMap<qint64, QString> lyrics;
    qint64 position;
    QString current;
};

#endif // DIALOGSHOWLYRICS_H
