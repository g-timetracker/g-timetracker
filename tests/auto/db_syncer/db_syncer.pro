CONFIG += testcase
CONFIG += parallel_test
TARGET = tst_db_syncer
QT += testlib quick sql
SOURCES  += tst_db_syncer.cpp

# timetracker lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../src/lib/release/ -ltimetracker
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../src/lib/debug/ -ltimetracker
else:unix: LIBS += -L$$OUT_PWD/../../../src/lib/ -ltimetracker

INCLUDEPATH += $$PWD/../../../src/lib
DEPENDPATH += $$PWD/../../../src/lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/release/libtimetracker.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/debug/libtimetracker.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/release/timetracker.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/debug/timetracker.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/libtimetracker.a

# tst_common lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../common/release/ -ltst_common
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../common/debug/ -ltst_common
else:unix: LIBS += -L$$OUT_PWD/../../common/ -ltst_common

INCLUDEPATH += $$PWD/../../common
DEPENDPATH += $$PWD/../../common

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/release/libtst_common.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/debug/libtst_common.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/release/tst_common.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/debug/tst_common.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../common/libtst_common.a
