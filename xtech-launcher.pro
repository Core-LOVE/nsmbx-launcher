TEMPLATE = app
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += static

TARGET = xtech-launcher

win32 {
    LIBS += -static  -lmingw32 -lSDL2main
    mxe:{
        LIBS += -lSDL2 -lsamplerate
    } else {
        LIBS += -lSDL2-static
    }
    LIBS += -lversion -lopengl32 -ldbghelp -ladvapi32 -lole32 -loleaut32 -luuid \
            -lkernel32 -lwinmm -limm32 -lgdi32 -luser32 -lsetupapi -static-libgcc
}

unix {
    LIBS += -no-pie
    LIBS += -static-libgcc
    LIBS += $$system(pkg-config --static --libs sdl2)
    QMAKE_CFLAGS += -posix $$system(pkg-config --static --cflags sdl2)
}

QMAKE_CFLAGS += -std=c90
QMAKE_CFLAGS_RELEASE += -O3

INCLUDEPATH += lib res

win32: RC_FILE = res/nsmbx.rc

SOURCES += \
        lib/ini.c \
        src/app.c \
        src/main.c \
        src/menu.c

HEADERS += \
    lib/ini.h \
    src/app.h \
    src/menu.h
