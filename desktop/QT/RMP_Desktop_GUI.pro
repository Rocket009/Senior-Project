QT       += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
win32 {
DEFINES += WIN32
LIBS += -lole32 -luuid -lwindowscodecs -lpropsys
INCLUDEPATH += $${QT_SYSROOT}/include
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    processvolumeslider.cpp \
    serialhandler.cpp \
    settings.cpp \
    windowsaudiosessioncontroller.cpp

HEADERS += \
    iaudiosessioncapture.h \
    iaudiosessioncontroller.h \
    mainwindow.h \
    processvolumeslider.h \
    serialhandler.h \
    settings.h \
    windowsaudiosessioncontroller.h

FORMS += \
    mainwindow.ui \
    processvolumeslider.ui \
    settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    desktop_gui.qmodel
