#-------------------------------------------------
#
# Project created by QtCreator 2018-02-02T13:24:34
#
#-------------------------------------------------

QT       += core gui network webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Klondike
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    logger.cpp \
    MapPainter.cpp

HEADERS  += mainwindow.h \
    logger.h \
    MapPainter.h

FORMS    += mainwindow.ui
