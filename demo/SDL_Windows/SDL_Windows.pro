TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c

INCLUDEPATH += "C:\msys64\mingw32\include"

INCLUDEPATH += $$PWD/../../src
HEADERS += $$PWD/../../src/csc_tcol.h

LIBS += -L"C:\msys64\mingw32\lib"
LIBS += -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lm
