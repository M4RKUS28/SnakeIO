QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    game.cpp \
    gamefield.cpp \
    graphicsview.cpp \
    main.cpp \
    mainwindow.cpp \
    snake.cpp

HEADERS += \
    game.h \
    gamefield.h \
    graphicsview.h \
    mainwindow.h \
    snake.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += $$PWD/../GenNet
DEPENDPATH += $$PWD/../GenNet

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../GenNet/release/ -lGenNet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../GenNet/debug/ -lGenNet


INCLUDEPATH += $$PWD/../ViewNet
DEPENDPATH += $$PWD/../ViewNet


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../ViewNet/release/ -lViewNet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../ViewNet/debug/ -lViewNet





win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../Bibliotheken/MUpdater/release/ -lMUpdater
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Bibliotheken/MUpdater/debug/ -lMUpdater

INCLUDEPATH += $$PWD/../../Bibliotheken/MUpdater
DEPENDPATH += $$PWD/../../Bibliotheken/MUpdater

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../Bibliotheken/MUpdater/release/libMUpdater.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../Bibliotheken/MUpdater/debug/libMUpdater.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../Bibliotheken/MUpdater/release/MUpdater.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../Bibliotheken/MUpdater/debug/MUpdater.lib

RESOURCES += \
    resources.qrc
