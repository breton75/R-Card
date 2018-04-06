#-------------------------------------------------
#
# Project created by QtCreator 2018-02-02T17:12:51
#
#-------------------------------------------------

QT       += core gui network sql xml serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Emulator
TEMPLATE = app
#CONFIG += console
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
    sv_vessel.cpp \
    sv_area.cpp \
    sv_vesselsymbol.cpp \
    sv_mapobjects.cpp \
    ../../svlib/sv_settings.cpp \
    ../../svlib/sv_sqlite.cpp \
    sv_vesseleditor.cpp \
    sv_vesselmotioneditor.cpp \
    sv_vesselposition.cpp \
    geo.cpp \
    sv_gps.cpp \
    sv_ais.cpp \
    ../../svlib/sv_log.cpp \
    sv_lag.cpp \
    nmea.cpp \
    sv_serialeditor.cpp \
    sv_navtex.cpp \
    sv_navtexeditor.cpp

HEADERS  += mainwindow.h \
    types.h \
    sv_vessel.h \
    sv_area.h \
    sv_mapobjects.h \
    ../../svlib/sv_settings.h \
    ../../svlib/sv_sqlite.h \
    sv_vesseleditor.h \
    sv_vesselmotioneditor.h \
    sv_vesselposition.h \
    sql_defs.h \
    sv_idevice.h \
    geo.h \
    sv_gps.h \
    sv_ais.h \
    sv_vessel_graphics.h \
    ../../svlib/sv_log.h \
    sv_lag.h \
    sv_exception.h \
    nmea.h \
    sv_serialeditor.h \
    sv_navtex.h \
    sv_navtexeditor.h

FORMS    += mainwindow.ui \
    sv_vesseleditor.ui \
    sv_vesselmotioneditor.ui \
    sv_vesselposition.ui \
    sv_serialeditor.ui \
    sv_navtexeditor.ui

RESOURCES += \
    res.qrc

STATECHARTS +=
