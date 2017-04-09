#-------------------------------------------------
#
# Project created by QtCreator 2016-10-02T11:19:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MK8Studio
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    filebase.cpp \
    customdelegate.cpp \
    formats/bfres.cpp \
    imageview.cpp \
    formats/ftex.cpp \
    formats/dds.cpp \
    nodes/ftexnode.cpp \
    nodes/bfresnode.cpp \
    formats/gx2.cpp

HEADERS  += mainwindow.h \
    filebase.h \
    customdelegate.h \
    formats/bfres.h \
    imageview.h \
    formats/ftex.h \
    common.h \
    formats/dds.h \
    nodes/ftexnode.h \
    nodes/bfresnode.h \
    nodes/nodegroup.h \
    formats/gx2.h

FORMS += \
    mainwindow.ui
