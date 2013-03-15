SOURCES = marblenet.cpp
include(chipmunk.pri)
QT += network
linux-*: QT += dbus
