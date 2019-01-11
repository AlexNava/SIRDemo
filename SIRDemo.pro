TEMPLATE = app
CONFIG += console c++11
CONFIG -= app
CONFIG -= qt

DESTDIR = $$PWD/bin
OBJECTS_DIR = $$DESTDIR

LIBS += -lSDL
#LIBS += -lSDLmain
LIBS += -lGL
LIBS += -lGLU
LIBS += -lglut
LIBS += -lGLEW

SOURCES += src/Main.cpp

DISTFILES += \
    data/stereo.frag \
    data/stereo.vert


