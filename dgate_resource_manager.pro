#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T18:14:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dgate_resource_manager
TEMPLATE = app

INCLUDEPATH += 3pp

SOURCES += main.cpp\
        mainwindow.cpp \
    videosurface.cpp \
    flicplayer.cpp \
    qfileinterface.cpp

HEADERS  += mainwindow.h \
    flicplayer.h \
    sleeper.h \
    qfileinterface.h \
    videosurface.h

FORMS    += mainwindow.ui

DISTFILES +=


unix:!macx: LIBS += -L$$PWD/3pp/flic/ -lflic-lib

INCLUDEPATH += $$PWD/3pp
DEPENDPATH += $$PWD/3pp

unix:!macx: PRE_TARGETDEPS += $$PWD/3pp/flic/libflic-lib.a
