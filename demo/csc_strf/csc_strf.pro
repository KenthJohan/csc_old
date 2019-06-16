TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../src
HEADERS += $$PWD/../../src/csc_strf.h
HEADERS += $$PWD/../../src/csc_basic.h
VPATH +=
SOURCES += main.c
#LIBS +=
