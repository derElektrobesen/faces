#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>

#ifndef LEARNING_MODE
#   include "imageprocessor.h"
#else
#   include "trainingpreview.h"
#endif

#include "databaseengine.h"

#define CLASS_NAME      "com.ics.faces"
#define CLASS_VERSION   1, 0
#define CLASS_INFO      CLASS_NAME, CLASS_VERSION

#define REGISTER_QML_TYPE(type, name) \
    qmlRegisterType< type >(CLASS_INFO, name)
#define REGISTER_QML_TYPE_T(type) \
    REGISTER_QML_TYPE(type, #type)

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

#ifndef LEARNING_MODE
    REGISTER_QML_TYPE_T(ImageProcessor);
#else
    REGISTER_QML_TYPE_T(TrainingPreview);
#endif

    if (!DataBaseEngine::initialize_database()) {
        qDebug() << "Database initialization failed";
        return 1;
    }

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/" MAIN_QML_FILE)));

    return app.exec();
}
