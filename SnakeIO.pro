QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
win32:RC_ICONS += icons/icon.ico

CONFIG += c++17


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/main.cpp \
    src/core/game.cpp \
    src/core/gamefield.cpp \
    src/core/snake.cpp \
    src/config/inputconfig.cpp \
    src/ui/graphicsview.cpp \
    src/ui/mainwindow.cpp \
    src/ui/pvemainwindow.cpp \
    src/ui/startdialog.cpp

HEADERS += \
    src/core/game.h \
    src/core/gamefield.h \
    src/core/snake.h \
    src/config/inputconfig.h \
    src/ui/graphicsview.h \
    src/ui/mainwindow.h \
    src/ui/pvemainwindow.h \
    src/ui/startdialog.h

FORMS += \
    src/ui/mainwindow.ui \
    src/ui/pvemainwindow.ui \
    src/ui/setupdialog.ui \
    src/ui/startdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# Project source directories — allows flat #include "file.h" from any module
INCLUDEPATH += $$PWD/src/core $$PWD/src/ui $$PWD/src/config

INCLUDEPATH += $$PWD/libs/GenNet/src
DEPENDPATH += $$PWD/libs/GenNet/src

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/GenNet/release/ -lGenNet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/GenNet/debug/ -lGenNet


INCLUDEPATH += $$PWD/libs/ViewNet
DEPENDPATH += $$PWD/libs/ViewNet


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/ViewNet/release/ -lViewNet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/ViewNet/debug/ -lViewNet





win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/MUpdaterLib/release/ -lMUpdater
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/MUpdaterLib/debug/ -lMUpdater

INCLUDEPATH += $$PWD/libs/MUpdaterLib/src
DEPENDPATH += $$PWD/libs/MUpdaterLib/src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/release/libMUpdater.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/debug/libMUpdater.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/release/MUpdater.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/debug/MUpdater.lib

RESOURCES += \
    resources.qrc
