#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>

#include "imageprocessor.h"

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

    REGISTER_QML_TYPE_T(ImageProcessor);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
