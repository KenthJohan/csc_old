TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += textgen.c
SOURCES += $$PWD/../../argparse/argparse.c

HEADERS += $$PWD/../../argparse/argparse.h

INCLUDEPATH += $$PWD/../../argparse
