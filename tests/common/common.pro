QT += testlib

TARGET = tst_common
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    tst_common.cpp

HEADERS += \
    tst_common.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

# timetracker lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/lib/release/ -ltimetracker
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/lib/debug/ -ltimetracker
else:unix: LIBS += -L$$OUT_PWD/../../src/lib/ -ltimetracker

INCLUDEPATH += $$PWD/../../src/lib
DEPENDPATH += $$PWD/../../src/lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../src/lib/release/libtimetracker.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../src/lib/debug/libtimetracker.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../src/lib/release/timetracker.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../src/lib/debug/timetracker.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../src/lib/libtimetracker.a
