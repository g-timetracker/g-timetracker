TEMPLATE = app

QT += qml quick widgets sql

SOURCES += main.cpp \
    TimeLogEntry.cpp \
    TimeLogModel.cpp \
    TimeLogData.cpp \
    ReverseProxyModel.cpp \
    DataImporter.cpp \
    TimeLogHistoryWorker.cpp \
    TimeLogHistory.cpp \
    TimeLog.cpp \
    TimeLogRecentModel.cpp \
    TimeLogSearchModel.cpp \
    DataExporter.cpp \
    AbstractDataInOut.cpp \
    TimeLogSyncData.cpp \
    DataSyncer.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    TimeLogEntry.h \
    TimeLogModel.h \
    TimeLogData.h \
    ReverseProxyModel.h \
    DataImporter.h \
    TimeLogHistoryWorker.h \
    TimeLogHistory.h \
    TimeLogHistory_p.h \
    TimeLog.h \
    TimeLog_p.h \
    TimeLogRecentModel.h \
    TimeLogSearchModel.h \
    DataExporter.h \
    AbstractDataInOut.h \
    TimeLogSyncData.h \
    DataSyncer.h \
    DataSyncer_p.h
