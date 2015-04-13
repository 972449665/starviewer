# Fitxer generat pel gestor de qmake de kdevelop. 
# ------------------------------------------- 
# Subdirectori relatiu al directori principal del projecte: ./src/extensions/playground/q2dviewer
# L'objectiu �s una biblioteca:

FORMS += q2dviewerextensionbase.ui \
         qhangingprotocolsgroupwidgetbase.ui

HEADERS += q2dviewerextensionmediator.h \
        q2dviewerextension.h \
        q2dviewersettings.h \
        layoutmanager.h \
        gridicon.h \
        tablemenu.h \
        itemmenu.h \
        qhangingprotocolswidget.h \
        qhangingprotocolsgroupwidget.h

SOURCES += q2dviewerextensionmediator.cpp \
        q2dviewerextension.cpp \
        q2dviewersettings.cpp \
        layoutmanager.cpp \
        gridicon.cpp \
        tablemenu.cpp \
        itemmenu.cpp \
        qhangingprotocolswidget.cpp \
        qhangingprotocolsgroupwidget.cpp

INCLUDEPATH += ../../../inputoutput
DEPENDPATH += ../../../inputoutput

RESOURCES += q2dviewer.qrc

EXTENSION_DIR = $$PWD
include(../../basicconfextensions.pri)
