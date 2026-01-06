QT += core gui widgets network

CONFIG += c++11

TARGET = cpftp-gui
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/gui/main_gui.cpp \
    src/gui/mainwindow.cpp \
    src/gui/connectiondialog.cpp \
    src/core/ftp_client.cpp \
    src/core/ftp_server.cpp \
    src/core/ftp_protocol.cpp \
    src/net/socket.cpp \
    src/utils/utility.cpp

HEADERS += \
    src/gui/mainwindow.h \
    src/gui/connectiondialog.h \
    src/core/ftp_client.h \
    src/core/ftp_server.h \
    src/core/ftp_protocol.h \
    src/net/socket.h \
    src/net/socket_exception.h \
    src/utils/utility.h

INCLUDEPATH += \
    src/gui \
    src/core \
    src/net \
    src/utils

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
