#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T18:14:51
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dgate_resource_manager
TEMPLATE = app

INCLUDEPATH += 3pp

SOURCES += main.cpp\
        mainwindow.cpp \
    videosurface.cpp \
    players/flicplayer.cpp \
    players/qiodeviceinterface.cpp \
    models/video.cpp \
    models/music.cpp \
    models/voice.cpp \
    players/xmiplayer.cpp \
    players/waveplayer.cpp \
    models/abstractaudiomodel.cpp \
    models/text.cpp

HEADERS  += mainwindow.h \
    videosurface.h \
    players/flicplayer.h \
    players/qiodeviceinterface.h \
    sleeper.h \
    models/video.h \
    models/music.h \
    models/voice.h \
    players/xmiplayer.h \
    players/waveplayer.h \
    models/trackinfo.h \
    models/abstractaudiomodel.h \
    models/text.h

FORMS    += mainwindow.ui

DISTFILES +=


unix:!macx: LIBS += -L$$PWD/3pp/flic/ -lflic-lib

INCLUDEPATH += $$PWD/3pp
DEPENDPATH += $$PWD/3pp

unix:!macx: PRE_TARGETDEPS += $$PWD/3pp/flic/libflic-lib.a

unix:!macx: LIBS += -lWildMidi
