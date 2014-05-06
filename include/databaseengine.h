#ifndef DATABASEENGINE_H
#define DATABASEENGINE_H

#include <QObject>
#include <QtCore>
#include <QtSql>
#include "main.h"

class DataBaseEngine : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseEngine(QObject *parent = 0);

    static bool initialize_database();

signals:

public slots:

};

#endif // DATABASEENGINE_H
