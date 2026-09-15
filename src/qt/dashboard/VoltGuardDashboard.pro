QT += widgets

CONFIG += c++17

INCLUDEPATH += include

SOURCES += \
	main.cpp \
	src/backendprocess.cpp \
	src/dashboardtypes.cpp \
	src/mainwindow.cpp

HEADERS += \
	include/backendprocess.h \
	include/dashboardtypes.h \
	include/mainwindow.h

TARGET = VoltGuardDashboard
