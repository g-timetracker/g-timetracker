TEMPLATE = app
TARGET = timetracker

QT += widgets qml quick sql

SOURCES += \
    main.cpp \
    FileLogger.cpp \

HEADERS += \
    FileLogger.h \

RESOURCES += qml.qrc \
    translations.qrc

lupdate_only {
SOURCES += $$PWD/qml/timetracker/*.qml \
           $$PWD/qml/timetracker/*.js
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =$$PWD/qml/

# Default rules for deployment.
include($$PWD/../deployment.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/ -ltimetracker
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/ -ltimetrackerd
else:unix: LIBS += -L$$OUT_PWD/../lib/ -ltimetracker

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/libtimetracker.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/libtimetrackerd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/timetracker.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/timetrackerd.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../lib/libtimetracker.a
