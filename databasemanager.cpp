#include "databasemanager.h"
#include "qurl.h"

DatabaseManager *DatabaseManager::instance = NULL;

DatabaseManager::DatabaseManager()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
}

void DatabaseManager::initDatabase()
{
    QString sql = "create table if not exists ";
    sql += "tb_playlist";
    sql += "(id integer primary key autoincrement, "
           "name text,"
           "url text unique not null,"
           "position integer,"
           "current integer default 0);";
    QSqlQuery query;
    if(!query.exec(sql))
    {
        const QSqlError &error = query.lastError();
        qDebug() << error.text();
    }
}

DatabaseManager *DatabaseManager::getInstance()
{
    if(NULL == instance)
    {
        instance = new DatabaseManager;
    }
    return instance;
}

bool DatabaseManager::createConnect()
{
    db.setDatabaseName("player.db");
    return db.open();
}

bool DatabaseManager::destoryConnect()
{
    db.close();
    return true;
}

void DatabaseManager::recordCurrent(const QUrl &url,int position)
{
    QSqlQuery query;
    query.prepare("insert into tb_playlist(name,url,position,current) values(:name, :url, :position, 1)");
    query.bindValue(":name",url.fileName());
    query.bindValue(":url",url);
    query.bindValue(":position",position);
    if(!query.exec())
    {
        const QSqlError &error = query.lastError();
        qDebug() << error.text();
    }

}

void DatabaseManager::recordList(const QUrl &url)
{
    QSqlQuery query;
    query.prepare("insert into tb_playlist(name,url) values(:name, :url)");
    query.bindValue(":name",url.fileName());
    query.bindValue(":url",url.toString());
    if(!query.exec())
    {
        const QSqlError &error = query.lastError();
        qDebug() << error.text();
    }
}


void DatabaseManager::deleteData()
{
    QSqlQuery query;
    query.prepare("delete from tb_playlist");
    if(query.exec())
    {
        qDebug() << "删除成功";
    }
    else
    {
       qDebug() << "删除失败";
    }
}

