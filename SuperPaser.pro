QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

equals(QT_MAJOR_VERSION, 5): lessThan(QT_MINOR_VERSION, 14): QMAKE_CXXFLAGS += -Wno-deprecated-copy

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    datainputwindow.cpp \
    descobj.cpp \
    main.cpp \
    mainwindow.cpp \
    structviewwindow.cpp \
    tableview.cpp \
    templateeditwindow.cpp \
    templatemanagewindow.cpp \
    texteditor.cpp

HEADERS += \
    datainputwindow.h \
    descobj.h \
    mainwindow.h \
    structviewwindow.h \
    tableview.h \
    templateeditwindow.h \
    templatemanagewindow.h \
    texteditor.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
