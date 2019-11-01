TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

HEADERS += "C:/Program Files (x86)/Kvaser/Canlib/INC/canlib.h"
HEADERS += ../../iup-3.27_Win64_mingw6_lib/include/iup.h
HEADERS += ../../cd-5.12_Win64_mingw6_lib/include/cd.h
HEADERS += ../../src/csc_basic.h
HEADERS += ../../src/csc_kvasercan.h
HEADERS += ../../src/csc_iup.h
HEADERS += ../../src/csc_debug.h
HEADERS += ../../src/csc_tcol.h
HEADERS += ../../src/csc_pacton.h
HEADERS += ../../src/csc_str.h
HEADERS += misc.h


INCLUDEPATH += ../../src
INCLUDEPATH += ../../iup-3.27_Win64_mingw6_lib/include
INCLUDEPATH += ../../cd-5.12_Win64_mingw6_lib/include
INCLUDEPATH += "C:\Program Files (x86)\Kvaser\Canlib\INC"

LIBS += -L../../iup-3.27_Win64_mingw6_lib
LIBS += -L../../cd-5.12_Win64_mingw6_lib
LIBS += -L"C:\Program Files (x86)\Kvaser\Canlib\Lib\x64"
#LIBS += -lcanlib32 -liup -lgdi32 -lcomdlg32 -lcomctl32 -luuid -loleaut32 -lole32
LIBS += -lcanlib32 -liup -liupcontrols -liupcd -lcd -lcdim -lcdgl -liupimglib -lfreetype6 -lz -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32
