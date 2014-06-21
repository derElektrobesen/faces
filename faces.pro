VERSION=1.0.0
VERSTR='\\"$${VERSION}\\"'
DB_F_NAME="faces.db3"

LEARNING_MODE=

DEFINES += \
    VER=\"$${VERSTR}\" \
    DEFAULT_RECO_FACTOR=1.2 \
    MAX_RECO_WND=120 \
    DEFAULT_FACE_RECT_SIZE=128 \
    STORE_INTERMIDATE_IMAGES \
    USE_HIGH_LEVEL_CONVERSION \
    STORE_NNET_IN_F \
    DEBUG

contains(DEFINES, STORE_INTERMIDATE_IMAGES) {
    DEFINES += INTERMIDATE_IMAGES_PATH='\\"/tmp/faces_results/\\"'
}

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
    QMAKE_CXXFLAGS += -std=c++11 -lmysqlclient_r

    DEFINES += WORK_DIR='\\"$${PWD}/database\\"'
}

contains(DEFINES, USE_MYSQL) {
    DEFINES += SQL_MAX_INSERT=2500
} else {
    DEFINES += SQL_MAX_INSERT=250
}

QT += qml quick multimedia core quick widgets sql

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

CONFIG += mobility qdeclarative-boostable qt-components
MOBILITY += multimedia

# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += include include/fann

HEADERS += \
    include/databaseengine.h \
    include/main.h \
    include/imageprovider.h \
    include/facerecognizer.h \
    include/haarcascade.h \
    include/fann/fann.h \
    include/fann/floatfann.h

SOURCES += src/main.cpp \
    src/databaseengine.cpp \
    src/imageprovider.cpp \
    src/facerecognizer.cpp \
    src/haarcascade.cpp \
    src/fann/floatfann.c \
    src/filters.cpp

OTHER_FILES += \
    rules.pri
