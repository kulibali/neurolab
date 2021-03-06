CONFIG += debug_and_release
QT += gui
QT += svg

TARGET = neurogui
TEMPLATE = lib

CONFIG += resources

macx { CONFIG += lib_bundle }

include(../version.txt)

DEFINES += NEUROGUI_LIBRARY

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

SOURCES += mainwindow.cpp \
    filedirtydialog.cpp \
    aboutdialog.cpp \
    labdatafile.cpp \
    labnetwork.cpp \
    labscene.cpp \
    labview.cpp \
    labtree.cpp \
    propertyobj.cpp \
    neuroitem.cpp \
    neuronetworkitem.cpp \
    narrow/neuronodeitem.cpp \
    narrow/neurolinkitem.cpp \
    narrow/neuronarrowitem.cpp \
    misc/neurotextitem.cpp \
    subnetwork/subnetworkitem.cpp \
    subnetwork/subconnectionitem.cpp \
    mixins/mixinarrow.cpp \
    mixins/mixinremember.cpp \
    compact/compactitem.cpp \
    compact/compactnodeitem.cpp \
    compact/compactoritem.cpp \
    compact/compactanditem.cpp \
    compact/compactlinkitem.cpp \
    itemslistwidget.cpp

HEADERS += neurogui_global.h \
    mainwindow.h \
    filedirtydialog.h \
    aboutdialog.h \
    labdatafile.h \
    labnetwork.h \
    labscene.h \
    labview.h \
    labtree.h \
    propertyobj.h \
    neuroitem.h \
    neuronetworkitem.h \
    narrow/neuronodeitem.h \
    narrow/neurolinkitem.h \
    narrow/neuronarrowitem.h \
    misc/neurotextitem.h \
    subnetwork/subnetworkitem.h \
    subnetwork/subconnectionitem.h \
    mixins/mixinarrow.h \
    mixins/mixinremember.h \
    compact/compactitem.h \
    compact/compactnodeitem.h \
    compact/compactoritem.h \
    compact/compactanditem.h \
    compact/compactlinkitem.h \
    itemslistwidget.h

FORMS += mainwindow.ui \
    filedirtydialog.ui \
    aboutdialog.ui

RESOURCES += neurogui.qrc

CONFIG(release, debug|release) { BUILDDIR=release }
CONFIG(debug, debug|release) {
    BUILDDIR=debug
    DEFINES += DEBUG
}

macx { DESTDIR = $$OUT_PWD/../$$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$OUT_PWD/../$$BUILDDIR }
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR

win32 {
    LIBS += -L$$DESTDIR \
        -lcommon1 \
        -lneurolib1 \
        -lautomata1 \
        -lqtpropertybrowser2
} else:macx {
    QMAKE_LFLAGS += -F$$DESTDIR
    LIBS += -framework common \
        -framework neurolib \
        -framework automata \
        -framework qtpropertybrowser
} else {
    LIBS += -L$$DESTDIR \
        -lcommon \
        -lneurolib \
        -lautomata \
        -lqtpropertybrowser
}
