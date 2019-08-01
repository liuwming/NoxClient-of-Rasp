#-------------------------------------------------
#
# Project created by QtCreator 2019-04-01T14:46:13
#
#-------------------------------------------------

QT       += core gui
CONFIG   += x86

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NoxClient
TEMPLATE = app

DESTDIR = $$OUT_PWD/bin
MOC_DIR = $$OUT_PWD/build_$$TARGET
RCC_DIR = $$OUT_PWD/build_$$TARGET
UI_DIR = $$OUT_PWD/build_$$TARGET
OBJECTS_DIR = $$OUT_PWD/build_$$TARGET


INCLUDEPATH += $$PWD/lib
DEPENDPATH += $$PWD/lib

LIBS += -L$${DESTDIR}/lib -l_rtt # 解决可执行程序运行时找不到路径的问题
PRE_TARGETDEPS += $${DESTDIR}/lib/lib_rtt.a

QMAKE_CXX += -DLOG_RESULT

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    nox/noxClient.cpp \
    ParamSet/ParamSet.cpp

HEADERS  += mainwindow.h \
    nox/noxClient.h \
    ParamSet/ParamSet.h

FORMS    += mainwindow.ui
