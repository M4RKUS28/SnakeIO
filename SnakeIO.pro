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




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/release/ -lGenNet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Debug/debug/ -lGenNet
else:unix: LIBS += -L$$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/ -lGenNet

INCLUDEPATH += $$PWD/../GenNet
DEPENDPATH += $$PWD/../GenNet

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/release/libGenNet.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Debug/debug/libGenNet.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/release/GenNet.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Debug/debug/GenNet.lib
else:unix: PRE_TARGETDEPS += $$PWD/../build-GenNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/libGenNet.a




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/release/ -lViewNet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Debug/debug/ -lViewNet
else:unix: LIBS += -L$$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/ -lViewNet

INCLUDEPATH += $$PWD/../ViewNet
DEPENDPATH += $$PWD/../ViewNet

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/release/libViewNet.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Debug/debug/libViewNet.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/release/ViewNet.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Debug/debug/ViewNet.lib
else:unix: PRE_TARGETDEPS += $$PWD/../build-ViewNet-Desktop_Qt_6_5_0_MinGW_64_bit-Release/libViewNet.a
