VERSION=1.0.0
VERSTR='\\"$${VERSION}\\"'
DB_F_NAME="faces.db3"

LEARNING_MODE=

DEFINES += \
    VER=\"$${VERSTR}\" \
    DEFAULT_RECO_FACTOR=1.2 \
    CLEAN_DB \
    DEBUG

include(rules.pri)

TEMPLATE = app

android {
    LIBS += -L/opt/android-ndk/necessitas/android-ndk/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/lib/gcc/arm-linux-androideabi/4.4.3/armv7-a/
    QT += androidextras

    LIBS += -L/usr/lib32/

    DEFINES += WORK_DIR='\\"/sdcard/.faces\\"' \
        CAMERA_ENABLED
}

linux {
    QMAKE_CXXFLAGS += -std=c++11

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
    include/imageprovider.h \
    include/facerecognizer.h \
    include/haarcascade.h

SOURCES += src/main.cpp \
    src/databaseengine.cpp \
    src/imageprovider.cpp \
    src/facerecognizer.cpp \
    src/haarcascade.cpp

OTHER_FILES += \
    rules.pri
