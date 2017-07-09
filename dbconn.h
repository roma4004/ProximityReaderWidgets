#ifndef DBCONN_H
#define DBCONN_H
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

static bool createConnSQLite()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Невозможно открыть базу"),
            qApp->tr("Невозможно установить соединение с базой.\n\n"
                     "Нажмите Cancel для выхода."), QMessageBox::Cancel);
        return false;
    }

    QSqlQuery query;

    query.exec("CREATE TABLE proximityCards("
               "card_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "card_number INTEGER, "
               "creation_date TEXT, "
               "name TEXT)");

    query.exec("INSERT INTO proximityCards VALUES("
               "NULL, "
               "0123456789, "
               "datetime('now'), "
               "'test name')");

    return true;
}

#endif // DBCONN_H
