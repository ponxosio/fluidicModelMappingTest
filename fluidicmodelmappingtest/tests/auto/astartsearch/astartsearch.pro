#-------------------------------------------------
#
# Project created by QtCreator 2017-03-30T11:48:44
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_astartsearchtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_astartsearchtest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

debug {
    INCLUDEPATH += X:\fluidicMachineModel\dll_debug\include
    LIBS += -L$$quote(X:\fluidicMachineModel\dll_debug\bin) -lFluidicMachineModel

    INCLUDEPATH += X:\utils\dll_debug\include
    LIBS += -L$$quote(X:\utils\dll_debug\bin) -lutils

    INCLUDEPATH += X:\commomModel\dll_debug\include
    LIBS += -L$$quote(X:\commomModel\dll_debug\bin) -lcommonModel

    INCLUDEPATH += X:\protocolGraph\dll_debug\include
    LIBS += -L$$quote(X:\protocolGraph\dll_debug\bin) -lprotocolGraph

    INCLUDEPATH += X:\fluidicModelMapping\dll_debug\include
    LIBS += -L$$quote(X:\fluidicModelMapping\dll_debug\bin) -lFluidicModelMapping

    INCLUDEPATH += X:\constraintsEngine\dll_debug\include
    LIBS += -L$$quote(X:\constraintsEngine\dll_debug\bin) -lconstraintsEngineLibrary
}

!debug {
    INCLUDEPATH += X:\fluidicMachineModel\dll_release\include
    LIBS += -L$$quote(X:\fluidicMachineModel\dll_release\bin) -lFluidicMachineModel

    INCLUDEPATH += X:\utils\dll_release\include
    LIBS += -L$$quote(X:\utils\dll_release\bin) -lutils

    INCLUDEPATH += X:\commomModel\dll_release\include
    LIBS += -L$$quote(X:\commomModel\dll_release\bin) -lcommonModel

    INCLUDEPATH += X:\protocolGraph\dll_release\include
    LIBS += -L$$quote(X:\protocolGraph\dll_release\bin) -lprotocolGraph

    INCLUDEPATH += X:\fluidicModelMapping\dll_release\include
    LIBS += -L$$quote(X:\fluidicModelMapping\dll_release\bin) -lFluidicModelMapping

    INCLUDEPATH += X:\constraintsEngine\dll_release\include
    LIBS += -L$$quote(X:\constraintsEngine\dll_release\bin) -lconstraintsEngineLibrary
}

INCLUDEPATH += X:\libraries\cereal-1.2.2\include

INCLUDEPATH += X:\swipl\include
LIBS += -L$$quote(X:\swipl\bin) -llibswipl
LIBS += -L$$quote(X:\swipl\lib) -llibswipl
