#include "worker.h"
#include <QDir>
Worker::Worker(QObject *parent)
    : QObject{parent}
{

}
Worker::~Worker()
{

}

Song* Worker::sendSong()
{
    return song;
}

void Worker::readLyrics(Song * song)   //解析歌词文件内容
{
    if (!song || song->url().isEmpty())
    {
        qDebug() << "歌曲不存在，或者歌曲文件路径为空";
        return;
    }

    qDebug() <<""<< *song;
    qDebug() << "解析歌词";
    QUrl mp3Url = song->url();
    QString lrcFile = mp3Url.toLocalFile().replace(".mp3", ".lrc");

    bool ret = QFileInfo(lrcFile).isFile();
    if (!ret)
    {
        qDebug() << "歌词文件不存在: " << lrcFile;
        return;
    }

    qDebug() << "打开歌词文件：" << lrcFile;
    QFile qfile(lrcFile);
    ret = qfile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ret)
    {
        qDebug() << "歌词文件打开失败: " << lrcFile;
        return;
    }

    //使用QTextStream按行遍历文件，读取每行内容
    QTextStream stream(&qfile);
    QString line;
    QString text;
    QStringList lineContents;   //歌词行分割后的内容数据:   时间戳    歌词文本
    QStringList timeContents;   //时间戳分割后的内容数组:   [分钟数   秒数.毫秒数
    QMap<qint64, QString> lyrics;   //存储歌词容器

    while (!stream.atEnd()) //循环读取，直到结束
    {
        line = stream.readLine();
        qDebug() << "line: " << line;
        if (line.isEmpty()) { continue; }

        //解析歌词信息
        lineContents = line.split(']');             //"时间戳"    "歌词文本"
        timeContents = lineContents[0].split(':');  //"[分钟数"   "秒数.毫秒数"

        //排除非歌词的信息行，将歌词行中的"[分钟数"转换成整型数值，如果转换失败说明不是歌词行
        bool ok = false;    //获取转换是否成功
        text = timeContents[0].mid(1);
        int minutes = text.toInt(&ok);  //字符串转数值，传入参数接收是否转成功
        if (!ok) { continue; }

        double seconds = timeContents[1].toDouble();    //字符串转浮点数
        lyrics.insert((minutes * 60 + seconds) * 1000, lineContents[1]);
        qDebug() << "新增歌词：" << (minutes * 60 + seconds) * 1000 << " " << lineContents;
    }

    qfile.close();
    qDebug() << "解析歌词结束，存储到歌曲对象中，歌词行数：" << lyrics.size();
    song->lyrics(lyrics);
}

void Worker::getASong(const QUrl & mp3Url)   //获取歌词文件内容
{
    qDebug() <<mp3Url.toLocalFile();
    QFileInfo info(mp3Url.toLocalFile());
    if (!info.isFile())
    {
        qDebug() << "不可用的mp3文件路径：" << mp3Url;
        emit getASongFailed();
        return;
    }
    //qDebug() << "将路径后缀.mp3替换为.lrc, 然后判断是否存在歌词文件";
    QString lrcFile = mp3Url.toLocalFile().replace(".mp3", ".lrc");
    bool ret = QFileInfo(lrcFile).isFile();
    if (!ret)
    {
        qDebug() << "歌词文件不存在: " << lrcFile;
        //qDebug() << "存储到消息队列并发出信号，生成的歌曲：" << *song;
        //MessageQueue::getInstance().push(song);
        emit getASongFailed();
        return;
    }
    qDebug() << "打开歌词文件";
    QFile qfile(lrcFile);
    ret = qfile.open(QIODevice::ReadOnly | QIODevice::Text); //只读模式加文本模式打开文件
    if (!ret)
    {
        //qDebug() << "打开文件失败: " << lrcFile;
        //qDebug() << "存储到消息队列并发出信号，生成的歌曲：" << *song;
        //MessageQueue::getInstance().push(song);
        emit getASongFailed();
        return;
    }
    //qDebug() << "构造一个歌曲对象";
    Song * song = new Song(mp3Url, info.baseName(), "", "");
    qDebug() << "读取歌词文件，按行读取，解析并存储歌名、歌手名和专辑名";
    //使用QTextStream按行遍历文件，读取每行内容
    QTextStream stream(&qfile);
    QString line;   //存储一行数据
    QString text;

    while (!stream.atEnd()) //循环读取，直到读到末尾结束
    {
        //读取一行数据，如果中文乱码，考虑使用数据库课件案例中的utf8ToGbk/gbkToUtf8进行转换
        line = stream.readLine();
        qDebug() << "line: " << line;
        if (line.isEmpty()) { continue; }

        //解析歌名、歌手、专辑
        if (line.startsWith("[ti:"))    //如果以[ti:开头说明后面是歌曲名
        {
            text = line.mid(4); //截取偏移量4开始的子串
            text.chop(1);       //去掉最后一个字符']'
            song->name(text);   //存储到歌曲对象中
            qDebug() << "读取并存储歌名：" << text;
        }
        else if (line.startsWith("[ar:"))
        {
            text = line.mid(4);
            text.chop(1);
            song->artist(text);
            qDebug() << "读取并存储歌手：" << text;
        }
        else if (line.startsWith("[al:"))
        {
            text = line.mid(4);
            text.chop(1);
            song->album(text);
            qDebug() << "读取并存储专辑：" << text;
        }
    }

    qfile.close();

    //歌词解析，单独封装一个解析函数
    readLyrics(song);

    qDebug() << "歌曲解析结束，生成歌曲：" << *song;
    this->song = song;
    emit getASongFinished();
}
