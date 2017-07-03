#-------------------------------------------------
#
# Project created by QtCreator 2016-10-02T11:19:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MK8Studio
TEMPLATE = app

SOURCES +=\
    MainWindow.cpp \
    Main.cpp \
    ImageView.cpp \
    CustomDelegate.cpp \
    Nodes/BFRESNode.cpp \
    Nodes/FTEXNode.cpp \
    Nodes/BFRESGroupNode.cpp \
    Formats/FTEX.cpp \
    Formats/DDS.cpp \
    Formats/BFRES.cpp \
    Formats/GX2ImageBase.cpp \
    Nodes/Node.cpp \
    IODialog.cpp \
    Formats/FormatBase.cpp \
    File.cpp

HEADERS  += \
    Common.h \
    CustomDelegate.h \
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
    CustomStandardItem.h \
    NonCopyable.h \
    File.h

FORMS += \
    MainWindow.ui \
    IODialog.ui
