QT += gui
QT += testlib

TEMPLATE = lib
TARGET = neurogui

DEFINES += NEUROGUI_LIBRARY

include(../version.txt)

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += mainwindow.cpp \
    labnetwork.cpp \
    labscene.cpp \
    labview.cpp \
    labtree.cpp \
    neuronodeitem.cpp \
    neurolinkitem.cpp \
    neuroitem.cpp \
    labeldialog.cpp \
    propertyobj.cpp \
    filedirtydialog.cpp \
    neuronarrowitem.cpp \
    labdatafile.cpp \
    aboutdialog.cpp

HEADERS += neurogui_global.h \
    mainwindow.h \
    labnetwork.h \
    labscene.h \
    labview.h \
    labtree.h \
    neuronodeitem.h \
    neurolinkitem.h \
    neuroitem.h \
    labeldialog.h \
    propertyobj.h \
    filedirtydialog.h \
    neuronarrowitem.h \
    labdatafile.h \
    aboutdialog.h

FORMS += mainwindow.ui \
    labeldialog.ui \
    filedirtydialog.ui \
    aboutdialog.ui

debug:BUILDDIR=debug
else:BUILDDIR=release

DESTDIR = ../$$BUILDDIR
OBJECTS_DIR = $$BUILDDIR
MOC_DIR = $$BUILDDIR
UI_DIR = $$BUILDDIR

win32:LIBS = -L$$DESTDIR \
    -lneurolib0 \
    -lautomata0 \
    -lqtpropertybrowser2
else:LIBS = -L$$DESTDIR \
    -lneurolib \
    -lautomata \
    -lqtpropertybrowser
