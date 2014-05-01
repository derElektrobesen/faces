VERSION=1.0.0
VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"

TEMPLATE = app

android {
    LIBS += -L/opt/android-ndk/necessitas/android-ndk/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/lib/gcc/arm-linux-androideabi/4.4.3/armv7-a/
}

QT += qml quick multimedia core quick widgets

SOURCES += src/main.cpp \
    src/imageprocessor.cpp \
    src/imageprovider.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

CONFIG += mobility qdeclarative-boostable qt-components
MOBILITY += multimedia

# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += include

HEADERS += \
    include/imageprocessor.h \
    include/imageprovider.h
