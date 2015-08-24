TEMPLATE = app

QT += qml quick widgets sql

SOURCES += main.cpp \
    TimeLogEntry.cpp \
    TimeLogModel.cpp \
    TimeLogData.cpp \
    TimeLogSingleton.cpp \
    TimeLogHistory.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    TimeLogEntry.h \
    TimeLogModel.h \
    TimeLogData.h \
    TimeLogSingleton.h \
    TimeLogHistory.h
