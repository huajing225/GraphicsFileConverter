template = app

CONFIG += console debug_and_release build_all
DESTDIR = ../../bin
TARGET = brc2pov

DEFINES += _CRT_SECURE_NO_WARNINGS
DEFINES += _SCL_SECURE_NO_WARNINGS

SOURCES += main.cpp

