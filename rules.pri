#!/bin/bash

!isEmpty( LEARNING_MODE ) {
    DEFINES += LEARNING_MODE \
        MAIN_QML_FILE='\\"main_with_learning.qml\\"'
    SOURCES += src/trainingpreview.cpp
    HEADERS += include/trainingpreview.h
} else {
    DEFINES += MAIN_QML_FILE='\\"main.qml\\"'
    SOURCES += src/imageprocessor.cpp
    HEADERS += include/imageprocessor.h
}
