VERSION=1.0.0
VERSTR='\\"$${VERSION}\\"'
DB_F_NAME="faces.db3"

LEARNING_MODE=1

DEFINES += \
    VER=\"$${VERSTR}\" \
    CLEAN_DB \

include(rules.pri)

TEMPLATE = app

android {
    LIBS += -L/opt/android-ndk/necessitas/android-ndk/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/lib/gcc/arm-linux-androideabi/4.4.3/armv7-a/
    QT += androidextras

    LIBS += -L/usr/lib32/

    DEFINES += WORK_DIR='\\"/sdcard/.faces\\"'
}

linux {
    QMAKE_CXXFLAGS += -std=c++0x

    DEFINES += WORK_DIR='\\"$${PWD}/database\\"'
}

QT += qml quick multimedia core quick widgets sql

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

CONFIG += mobility qdeclarative-boostable qt-components
MOBILITY += multimedia

# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += include

HEADERS += \
    include/databaseengine.h \
    include/main.h \
    include/imageprovider.h

SOURCES += src/main.cpp \
    src/databaseengine.cpp \
    src/imageprovider.cpp

OTHER_FILES += \
    rules.pri
