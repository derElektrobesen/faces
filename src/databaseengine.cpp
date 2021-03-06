#include "databaseengine.h"

DataBaseEngine::DataBaseEngine(QObject *parent) :
    QObject(parent)
{
}

bool DataBaseEngine::initialize_database() {
#ifndef USE_MYSQL
    QFile db_file(":/assets/" DB_F_NAME);
    const char fname[] = WORK_DIR "/" DB_F_NAME;

#ifdef CLEAN_DB
    QFile(fname).remove();
#endif

    if (db_file.exists() && !(QFile(fname).exists())) {
        QDir().mkdir(WORK_DIR);
        db_file.copy(fname);
        QFile::setPermissions(fname, QFile::WriteOwner | QFile::ReadOwner);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DB_NAME);
    db.setDatabaseName(fname);
#else
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", DB_NAME);
    db.setUserName("user");
    db.setPassword("");
    db.setDatabaseName("face_recognizer");
#endif
    return db.open();
}
