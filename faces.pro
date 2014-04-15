TEMPLATE = app

android {
    LIBS += -L/opt/android-ndk/necessitas/android-ndk/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/lib/gcc/arm-linux-androideabi/4.4.3/armv7-a/
}

QT += qml quick multimedia core

SOURCES += main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

CONFIG += mobility
MOBILITY += multimedia

# Default rules for deployment.
include(deployment.pri)
