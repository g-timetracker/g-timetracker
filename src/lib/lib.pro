TEMPLATE = lib
TARGET = timetracker

CONFIG += staticlib

QT += quick sql

DEFINES *= QT_USE_QSTRINGBUILDER

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
    DataSyncerWorker.cpp \
    DataSyncer.cpp \
    TimeLogStats.cpp \
    TimeTracker.cpp \
    TimeLogCategoryTreeModel.cpp \
    TimeLogCategoryDepthModel.cpp \
    DBSyncer.cpp \
    TimeLogCategoryData.cpp \
    TimeLogSyncDataBase.cpp \
    TimeLogSyncDataCategory.cpp \
    TimeLogCategory.cpp \
    TimeLogCategoryTreeNode.cpp \
    TimeLogSyncDataEntry.cpp

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
    DataSyncerWorker.h \
    DataSyncer.h \
    TimeLogStats.h \
    TimeTracker.h \
    TimeLogCategoryTreeModel.h \
    TimeLogCategoryDepthModel.h \
    DBSyncer.h \
    TimeLogCategoryData.h \
    TimeLogSyncDataBase.h \
    TimeLogSyncDataCategory.h \
    TimeLogCategory.h \
    TimeLogCategoryTreeNode.h \
    TimeLogSyncDataEntry.h
