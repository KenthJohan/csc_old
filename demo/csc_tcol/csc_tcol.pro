TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../src
HEADERS += $$PWD/../../src/csc_tcol.h
VPATH +=
SOURCES += main.c
#LIBS +=