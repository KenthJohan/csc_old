TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../src
HEADERS += $$PWD/../../src/csc_tcol.h
HEADERS += $$PWD/../../src/csc_str.h
HEADERS += $$PWD/../../src/csc_tok_c.h
HEADERS += $$PWD/../../src/csc_tree4.h
VPATH +=
SOURCES += main.c

#IUP
INCLUDEPATH += $$PWD/../../iup-3.28_Win64_mingw6_lib/include
HEADERS += $$PWD/../../iup-3.28_Win64_mingw6_lib/include/iup.h
LIBS += -L$$PWD/../../iup-3.28_Win64_mingw6_lib
LIBS += -liup -lgdi32 -lcomdlg32 -lcomctl32 -luuid -loleaut32 -lole32

