CONFIG += testcase
CONFIG += parallel_test
TARGET = tst_sync_pack
QT += testlib quick sql
SOURCES  += tst_sync_pack.cpp

# timetracker lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../src/lib/ -ltimetracker
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../src/lib/ -ltimetrackerd
else:unix: LIBS += -L$$OUT_PWD/../../../src/lib/ -ltimetracker

INCLUDEPATH += $$PWD/../../../src/lib
DEPENDPATH += $$PWD/../../../src/lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/libtimetracker.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/libtimetrackerd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/timetracker.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/timetrackerd.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../../src/lib/libtimetracker.a

# tst_common lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../common/ -ltst_common
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../common/ -ltst_commond
else:unix: LIBS += -L$$OUT_PWD/../../common/ -ltst_common

INCLUDEPATH += $$PWD/../../common
DEPENDPATH += $$PWD/../../common

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/libtst_common.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/libtst_commond.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/tst_common.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../common/tst_commond.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../common/libtst_common.a
