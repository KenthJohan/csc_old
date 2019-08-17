TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


QMAKE_CFLAGS += -fprofile-arcs -ftest-coverage -O0
LIBS += -lgcov

SOURCES += ide.c

HEADERS += $$PWD/../../iup-3.27_Win64_mingw6_lib/include/iup.h
HEADERS += $$PWD/../../cd-5.12_Win64_mingw6_lib/include/cd.h
HEADERS += $$PWD/../../src/csc_debug.h
HEADERS += $$PWD/../../src/csc_debug_uv.h
HEADERS += $$PWD/../../src/csc_malloc_file.h
HEADERS += $$PWD/../../src/csc_fspath.h
HEADERS += $$PWD/../../src/csc_str.h
HEADERS += $$PWD/../../src/csc_iup.h
HEADERS += $$PWD/../../src/csc_realpath.h
HEADERS += img.h
HEADERS += sci.h
HEADERS += fstree.h

INCLUDEPATH += "C:/msys64/mingw64/include"
INCLUDEPATH += $$PWD/../../iup-3.27_Win64_mingw6_lib/include
INCLUDEPATH += $$PWD/../../cd-5.12_Win64_mingw6_lib/include
INCLUDEPATH += $$PWD/../../src

LIBS += -L$$PWD/../../iup-3.27_Win64_mingw6_lib
LIBS += -L$$PWD/../../cd-5.12_Win64_mingw6_lib
#LIBS += -liup -lgdi32 -lcomdlg32 -lcomctl32 -luuid -loleaut32 -lole32
LIBS += -liup_scintilla -liup -liupcontrols -liupcd -lcd -limm32 -lcdim -lcdgl -liupimglib -lfreetype6 -lz -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32

LIBS += -L"C:\msys64\mingw64\lib"
LIBS += -luv

