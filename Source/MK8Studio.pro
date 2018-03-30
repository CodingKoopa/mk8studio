#-------------------------------------------------
#
# Project created by QtCreator 2016-10-02T11:19:30
#
#-------------------------------------------------

QT       += core gui widgets
QMAKE_CXXFLAGS += -std=c++17

TARGET = MK8Studio
TEMPLATE = app

SOURCES +=\
    MainWindow.cpp \
    Main.cpp \
    CustomDelegate.cpp \
    Nodes/Node.cpp \
    IODialog.cpp \
    Formats/FormatBase.cpp \
    File.cpp \
    CustomStandardItem.cpp \
    Formats/Models/FMDL.cpp \
    Formats/Textures/GX2ImageBase.cpp \
    Formats/Textures/DDS.cpp \
    Formats/Textures/FTEX.cpp \
    Formats/Archives/BFRES.cpp \
    Nodes/Archives/BFRESNode.cpp \
    Nodes/Textures/FTEXNode.cpp \
    Nodes/Models/FMDLNode.cpp \
    Formats/Models/FVTX.cpp \
    Nodes/Models/FVTXNode.cpp \
    Nodes/Models/FVTXAttributeNode.cpp \
    Nodes/Archives/BFRESGroupNode.cpp \
    Nodes/Models/FVTXBufferNode.cpp

HEADERS  += \
    Common.h \
    CustomDelegate.h \
    MainWindow.h \
    Nodes/Node.h \
    Formats/FormatBase.h \
    IODialog.h \
    CustomStandardItem.h \
    File.h \
    Formats/Models/FMDL.h \
    Formats/Textures/GX2ImageBase.h \
    Formats/Textures/FTEX.h \
    Formats/Textures/DDS.h \
    Formats/Archives/BFRES.h \
    Nodes/Archives/BFRESGroupNode.h \
    Nodes/Archives/BFRESNode.h \
    Nodes/Textures/FTEXNode.h \
    Nodes/Models/FMDLNode.h \
    Formats/Models/FVTX.h \
    Nodes/Models/FVTXNode.h \
    Nodes/Models/FVTXAttributeNode.h \
    Formats/Common/ResourceDictionary.h \
    Nodes/Models/FVTXBufferNode.h

FORMS += \
    MainWindow.ui \
    IODialog.ui
