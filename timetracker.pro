TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp \
    TimeLogEntry.cpp \
    TimeLogModel.cpp \
    TimeLogData.cpp \
    TimeLogSingleton.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    TimeLogEntry.h \
    TimeLogModel.h \
    TimeLogData.h \
    TimeLogSingleton.h
