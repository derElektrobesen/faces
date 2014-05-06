#ifndef MAIN_H
#define MAIN_H

#include <QDebug>

#define DB_NAME     "faces"
#define DB_F_NAME   "faces.db3"

#define DECLARE_SQL_CON(var) QSqlQuery var(QSqlDatabase::database(DB_NAME))

#endif // MAIN_H
