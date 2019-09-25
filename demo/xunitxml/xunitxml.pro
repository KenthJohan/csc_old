TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../src
SOURCES += main.c xunit.c
INCLUDEPATH += $$PWD/../../mxml-3.1
LIBS += -L$$PWD/../../mxml-3.1
LIBS += -lmxml
HEADERS += $$PWD/xunit.h
