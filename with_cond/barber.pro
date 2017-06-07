TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    thread_queue.c \
    ulepszenia_terminala.c

LIBS += -pthread

HEADERS += bool.h \
    thread_queue.h \
    ulepszenia_terminala.h

