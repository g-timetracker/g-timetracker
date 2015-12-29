TEMPLATE = lib
TARGET = timetracker

CONFIG += staticlib

QT += quick sql

SOURCES += \
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
    TimeTracker.cpp \
    TimeLogCategory.cpp \
    TimeLogCategoryTreeModel.cpp \
    TimeLogCategoryDepthModel.cpp

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
    TimeTracker.h \
    TimeLogCategory.h \
    TimeLogCategoryTreeModel.h \
    TimeLogCategoryDepthModel.h
