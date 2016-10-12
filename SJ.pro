#-------------------------------------------------
#
# Project created by QtCreator 2016-10-11T00:30:28
#
#-------------------------------------------------

QT       += core gui xml
QMAKE_CXXFLAGS += -std=c++0x
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SJ
TEMPLATE = app


SOURCES += main.cpp\
        qgis_dev.cpp \
    qgis_devlayertreeviewmenuprovider.cpp \
    qgis_devmaptoolidentifyaction.cpp \
    qgis_dev_addfeaturetool.cpp

HEADERS  += qgis_dev.h \
    qgis_devlayertreeviewmenuprovider.h \
    qgis_devmaptoolidentifyaction.h \
    qgis_dev_addfeaturetool.h

FORMS    += qgis_dev.ui \
    qgsnewvectorlayerdialogbase.ui \
    qgsadvanceddigitizingdockwidgetbase.ui

unix{
DEFINES += CORE_EXPORT=
DEFINES += GUI_EXPORT=
DEFINES += ANALYSIS_EXPORT=
}
!unix{
DEFINES += CORE_EXPORT=__declspec(dllimport)
DEFINES += GUI_EXPORT=__declspec(dllimport)
}

LIBS += -lqgis_core -lqgis_gui -lqgis_analysis
INCLUDEPATH += /usr/include/qgis
