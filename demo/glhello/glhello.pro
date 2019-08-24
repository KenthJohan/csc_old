TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += win64

SOURCES += main.c
SOURCES += $$PWD/../../src/opengl46/src/glad.c



INCLUDEPATH += $$PWD/../../src
HEADERS += $$PWD/../../src/csc_tcol.h
HEADERS += $$PWD/../../src/xgl.h
HEADERS += $$PWD/../../src/csc_sdl.h
HEADERS += $$PWD/../../src/v.h
HEADERS += $$PWD/../../src/v3.h
HEADERS += $$PWD/../../src/v4.h
HEADERS += $$PWD/test.glfs
HEADERS += $$PWD/test.glvs

INCLUDEPATH += $$PWD/../../src/opengl46/include

win32:INCLUDEPATH += "C:\msys64\mingw32\include"
win32:LIBS += -L"C:\msys32\mingw32\lib"
win32:LIBS += -lmingw32 -lSDL2main -lSDL2 -mwindows  -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc

win64:INCLUDEPATH += "C:\msys64\mingw64\include"
win64:LIBS += -L"C:\msys64\mingw64\lib"
win64:LIBS += -lmingw32 -lSDL2main -lSDL2 -mwindows  -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc
