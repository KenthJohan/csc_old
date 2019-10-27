TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++17

#QMAKE_CXXFLAGS += -std=c++17

SOURCES += \
    main.c
     #main2.cpp



INCLUDEPATH += $$PWD/../../src
HEADERS += $$PWD/../../src/csc_tcol.h \
    misc.h
HEADERS += $$PWD/../../src/csc_debug.h
HEADERS += $$PWD/../../src/csc_malloc_file.h
HEADERS += $$PWD/../../src/csc_basic.h
HEADERS += $$PWD/../../src/csc_vk.h
HEADERS += $$PWD/../../src/csc_glfw.h
HEADERS += shader.vert
HEADERS += shader.frag

INCLUDEPATH += $$PWD/../../src/opengl46/include
INCLUDEPATH += C:/VulkanSDK/1.1.121.0/Include





contains(QT_ARCH, i386){
    message("Using mingw32 libs")
    INCLUDEPATH += C:/msys64/mingw32/include
    LIBS += -LC:/msys64/mingw32/lib
    LIBS += -LC:/VulkanSDK/1.1.114.0/Lib32
} else {
    message("Using mingw64 libs.")
    INCLUDEPATH += C:/msys64/mingw64/include
    LIBS += -LC:/msys64/mingw64/lib
    LIBS += -LC:/VulkanSDK/1.1.114.0/Lib
}

#LIBS += -static -lglfw3 -lglu32 -lopengl32 -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32
LIBS += -static -lglfw3 -lgdi32 -lvulkan-1
