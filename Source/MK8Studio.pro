#-------------------------------------------------
#
# Project created by QtCreator 2016-10-02T11:19:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -Wall -pedantic -std=c++14

TARGET = MK8Studio
TEMPLATE = app

SOURCES +=\
    MainWindow.cpp \
    Main.cpp \
    ImageView.cpp \
    FileBase.cpp \
    CustomDelegate.cpp \
    Nodes/BFRESNode.cpp \
    Nodes/FTEXNode.cpp \
    Nodes/BFRESGroupNode.cpp \
    Formats/FTEX.cpp \
    Formats/DDS.cpp \
    Formats/BFRES.cpp \
    Formats/GX2ImageBase.cpp \
    Nodes/Node.cpp \
    IODialog.cpp

HEADERS  += \
    Common.h \
    CustomDelegate.h \
    FileBase.h \
    ImageView.h \
    MainWindow.h \
    Nodes/Node.h \
    Nodes/FTEXNode.h \
    Nodes/BFRESNode.h \
    Nodes/BFRESGroupNode.h \
    Formats/FTEX.h \
    Formats/DDS.h \
    Formats/BFRES.h \
    Formats/FormatBase.h \
    Formats/GX2ImageBase.h \
    IODialog.h \
    CustomStandardItem.h

FORMS += \
    MainWindow.ui \
    IODialog.ui
