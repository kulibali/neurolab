QT += gui

TARGET = griditems
TEMPLATE = lib

macx { CONFIG += lib_bundle }
macx { CONFIG += x86 }
macx { CONFIG -= x86_64 }
else { QMAKE_CXXFLAGS += -Wno-type-limits }

include(../version.txt)

INCLUDEPATH += ../thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/src

DEFINES += GRIDITEMS_LIBRARY

SOURCES += neurogriditem.cpp

HEADERS += neurogriditem.h\
        griditems_global.h

release { BUILDDIR=release }
debug { BUILDDIR=debug }

macx { DESTDIR = $$OUT_PWD/../$$BUILDDIR/neurolab.app/Contents/Frameworks }
else { DESTDIR = $$OUT_PWD/../$$BUILDDIR/plugins }
TEMPDIR = $$OUT_PWD/$$BUILDDIR

OBJECTS_DIR = $$TEMPDIR
MOC_DIR = $$TEMPDIR
UI_DIR = $$TEMPDIR
RCC_DIR = $$TEMPDIR

win32 {
    LIBS += -L$$DESTDIR/.. \
        -lneurogui1 \
        -lneurolib1 \
        -lautomata1 \
        -lqtpropertybrowser2
} else:macx {
    QMAKE_LFLAGS += -F$$DESTDIR
    LIBS += -framework neurogui \
        -framework neurolib \
        -framework automata \
        -framework qtpropertybrowser
} else {
    LIBS += -L$$DESTDIR/.. \
        -lneurogui \
        -lneurolib \
        -lautomata \
        -lqtpropertybrowser
}
