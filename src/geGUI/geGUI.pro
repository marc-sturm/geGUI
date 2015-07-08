#-------------------------------------------------
#
# Project created by QtCreator 2013-02-23T15:29:44
#
#-------------------------------------------------

QT += core widgets gui xml xmlpatterns

TARGET = geGUI
TEMPLATE = app
RC_FILE	 = icon.rc

SOURCES += main.cpp\
    MainWindow.cpp \
    TDX.cpp \
    MultiFileChooser.cpp \
	ToolsDialog.cpp \

HEADERS  += MainWindow.h \
    TDX.h \
    MultiFileChooser.h \
	ToolsDialog.h \

FORMS    += MainWindow.ui \
    MultiFileChooser.ui \
    ToolsDialog.ui

#include cppCORE library
INCLUDEPATH += $$PWD/../../src/cppCORE
LIBS += -L$$PWD/../../bin -lcppCORE

#include cppGUI library
INCLUDEPATH += $$PWD/../../src/cppGUI
LIBS += -L$$PWD/../../bin -lcppGUI

#include cppXML library
INCLUDEPATH += $$PWD/../../src/cppXML
LIBS += -L$$PWD/../../bin -lcppXML

#copy EXE to bin folder
DESTDIR = $$PWD/../../bin

RESOURCES += \
    Resources.qrc
