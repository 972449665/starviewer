# Fitxer generat pel gestor de qmake de kdevelop. 
# ------------------------------------------- 
# Subdirectori relatiu al directori principal del projecte: ./src/extensions/playground/optimalviewpoint
# L'objectiu �s una biblioteca:  

FORMS += qmpr3dextensionbase.ui

HEADERS += qmpr3dextension.h \
           mpr3dsettings.h \
           mpr3dextensionmediator.h

SOURCES += qmpr3dextension.cpp \
           mpr3dsettings.cpp \
           mpr3dextensionmediator.cpp

RESOURCES += mpr3d.qrc

EXTENSION_DIR = $$PWD
include(../../basicconfextensions.inc)
