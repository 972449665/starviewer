# Fitxer generat pel gestor de qmake de kdevelop. 
# ------------------------------------------- 
# Subdirectori relatiu al directori principal del projecte: ./src
# L'objectiu és un subdirectori del projecte 

SUBDIRS += core \
           inputoutput \
           interface \
           extensions \
           main 
TEMPLATE = subdirs 
CONFIG += warn_on 
OBJECTS_DIR = ../tmp/obj 
UI_DIR = ../tmp/ui 
MOC_DIR = ../tmp/moc 
LIBS += core/core.a \
        inputoutput/libinputoutput.a \
        interface/libinterface.a
TARGETDEPS += ../src/main/../bin/starviewer \
              ../src/interface/libinterface.a \
              ../src/inputoutput/libinputoutput.a \
              ../src/core/core.a

include(compilationtype.inc)
