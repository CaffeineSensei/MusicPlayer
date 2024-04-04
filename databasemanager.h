#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseManager
{
public:
    DatabaseManager();
    void initDatabase();
    static DatabaseManager* getInstance();
    bool createConnect();
    bool destoryConnect();
    void recordCurrent(const QUrl &url,int position);
    void recordList(const QUrl &url);
    void deleteData();
private:
    static DatabaseManager* instance;
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
