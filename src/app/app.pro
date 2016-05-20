TEMPLATE = app
TARGET = g-timetracker

QT += qml quick sql

!android {
    QT += widgets
    QT += network
}

SOURCES += \
    main.cpp \
    FileLogger.cpp \
    Notifier.cpp

HEADERS += \
    FileLogger.h \
    Notifier.h

!android {
    SOURCES += Updater.cpp
    HEADERS += Updater.h
}

RESOURCES += qml.qrc \
    translations.qrc

!android {
    RESOURCES += fonts.qrc
}

RC_ICONS += $$PWD/icons/windows/app_icon.ico

lupdate_only {
SOURCES += $$PWD/qml/timetracker/*.qml \
           $$PWD/qml/timetracker/*.js
}

!android {
    include($$PWD/../3rdparty/qt-solutions/qtsingleapplication/src/qtsingleapplication.pri)
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =$$PWD/qml/

# Default rules for deployment.
include($$PWD/../deployment.pri)

# timetracker lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -ltimetracker
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -ltimetracker
else:unix: LIBS += -L$$OUT_PWD/../lib/ -ltimetracker

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/release/libtimetracker.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/debug/libtimetracker.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/release/timetracker.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/debug/timetracker.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../lib/libtimetracker.a

DISTFILES += \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    android/build.gradle

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
