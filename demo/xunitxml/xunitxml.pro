TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../src
INCLUDEPATH += $$PWD/../../mxml-3.1
INCLUDEPATH += $$PWD/../../liblfds711/inc

LIBS += -L$$PWD/../../mxml-3.1
LIBS += -L$$PWD/../../liblfds711/bin
LIBS += -lmxml -llfds711

SOURCES += main.c xunit.c

HEADERS += $$PWD/xunit.h
HEADERS += $$PWD/../../src/csc_readmisc.h
HEADERS += $$files($$PWD/../../liblfds711/inc/*.h, true)
