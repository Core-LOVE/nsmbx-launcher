TEMPLATE = app
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = xtech-launcher

!win32: LIBS += -no-pie
win32: LIBS += -lmingw32 -lSDL2main
LIBS += -lSDL2
!win32: QMAKE_CFLAGS += -posix
QMAKE_CFLAGS += -std=c90
QMAKE_CFLAGS_RELEASE += -O3

INCLUDEPATH += lib res

SOURCES += \
        lib/ini.c \
        src/app.c \
        src/main.c \
        src/menu.c

HEADERS += \
    lib/ini.h \
    src/app.h \
    src/menu.h
