#-------------------------------------------------
#
# Project created by QtCreator 2017-10-24T08:23:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GenieMaster
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    configdialog.cpp \
    hextestdialog.cpp \
    dtablemodel.cpp \
    dalarmthread.cpp \
    alarmmsgdlg.cpp \
    dhistroythread.cpp \
    historicaldatadlg.cpp \
    dstringlist.cpp \
    dscrollerlabel.cpp \
    daboutdlg.cpp \
    dhelpdialog.cpp

HEADERS  += mainwindow.h \
    configdialog.h \
    hexfloat.h \
    hextestdialog.h \
    dtablemodel.h \
    dalarmthread.h \
    alarmmsgdlg.h \
    dhistroythread.h \
    historicaldatadlg.h \
    dstringlist.h \
    dscrollerlabel.h \
    daboutdlg.h \
    dhelpdialog.h

FORMS    += mainwindow.ui \
    configdialog.ui \
    hextestdialog.ui \
    alarmmsgdlg.ui \
    historicaldatadlg.ui \
    daboutdlg.ui \
    dhelpdialog.ui

RC_FILE += re.rc

QT += serialport

RESOURCES += \
    resource.qrc

TRANSLATIONS += myapp.ts

