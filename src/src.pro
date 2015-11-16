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
    TimeLogRecentModel.cpp \
    TimeLogSearchModel.cpp \
    DataExporter.cpp \
    AbstractDataInOut.cpp \
    TimeLogSyncData.cpp \
    DataSyncerWorker.cpp \
    DataSyncer.cpp \
    TimeLogStats.cpp \
    FileLogger.cpp \
    TimeTracker.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =qml/

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
    TimeLogRecentModel.h \
    TimeLogSearchModel.h \
    DataExporter.h \
    AbstractDataInOut.h \
    TimeLogSyncData.h \
    DataSyncerWorker.h \
    DataSyncer.h \
    TimeLogStats.h \
    FileLogger.h \
    TimeTracker.h