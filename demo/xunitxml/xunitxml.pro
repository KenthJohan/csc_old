TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../src
INCLUDEPATH += $$PWD/../../mxml-3.1
INCLUDEPATH += $$PWD/../../liblfds711/inc
INCLUDEPATH += $$PWD/../../argparse

LIBS += -L$$PWD/../../mxml-3.1
LIBS += -L$$PWD/../../liblfds711/bin
LIBS += -lmxml -llfds711

SOURCES += main.c
SOURCES += $$PWD/../../argparse/argparse.c

HEADERS += qasync.h
HEADERS += threads.h
HEADERS += tsuite.h
HEADERS += $$PWD/../../src/csc_readmisc.h
HEADERS += $$PWD/../../argparse/argparse.h
HEADERS += $$files($$PWD/../../liblfds711/inc/*.h, true)
