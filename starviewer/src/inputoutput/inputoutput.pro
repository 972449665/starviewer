# Fitxer generat pel gestor de qmake de kdevelop. 
# ------------------------------------------- 
# Subdirectori relatiu al directori principal del projecte: ./src/inputoutput
# L'objectiu és una biblioteca:  

TRANSLATIONS += inputoutput_ca_ES.ts \
                inputoutput_es_ES.ts \
                inputoutput_en_GB.ts 
FORMS += qconfigurationscreenbase.ui \
         qpacslistbase.ui \
         qstudytreewidgetbase.ui \
         qserieslistwidgetbase.ui \
         qcreatedicomdirbase.ui \
         qoperationstatescreenbase.ui \
         queryscreenbase.ui \
         qadvancedsearchwidgetbase.ui \
         qbasicsearchwidgetbase.ui \
         qlocaldatabaseconfigurationscreenbase.ui \
         qlistenrisrequestsconfigurationscreenbase.ui
HEADERS += databaseconnection.h \
           dicomimage.h \
           pacsconnection.h \
           pacslistdb.h \
           pacsnetwork.h \
           pacsparameters.h \
           pacsserver.h \
           processimage.h \
           processimagesingleton.h \
           retrieveimages.h \
           dicomseries.h \
           starviewersettings.h \
           status.h \
           struct.h \
           dicomstudy.h \
           multiplequerystudy.h \
           qquerystudythread.h \
           operation.h \
           qexecuteoperationthread.h \
           converttodicomdir.h \
           convertdicomtolittleendian.h \
           createdicomdir.h \
           dicomdirreader.h \
           storeimages.h \
           starviewerprocessimagestored.h \
           starviewerprocessimageretrieved.h \
           querypacs.h \
           dicommask.h \
           dicomdirimporter.h \
           qconfigurationscreen.h \
           qpacslist.h \
           qstudytreewidget.h \
           qserieslistwidget.h \
           qcreatedicomdir.h \
           qoperationstatescreen.h \
           queryscreen.h \
 	   errordcmtk.h \
           qadvancedsearchwidget.h \
           qbasicsearchwidget.h \
           localdatabasemanager.h \
           localdatabaseimagedal.h \
           testdicomobjects.h \
           localdatabaseseriesdal.h \
           localdatabasestudydal.h \
           localdatabasepatientdal.h \
           localdatabaseutildal.h \
           testdatabase.h \
           qdeleteoldstudiesthread.h \
           localdatabasemanagerthreaded.h \
           qthreadrunwithexec.h \
           databaseinstallation.h \
           qlocaldatabaseconfigurationscreen.h \
           parsexmlrispierrequest.h \
           listenrisrequest.h \
           processrisrequestthread.h \
           qlistenrisrequestsconfigurationscreen.h

SOURCES += databaseconnection.cpp \
           dicomimage.cpp \
           pacsconnection.cpp \
           pacslistdb.cpp \
           pacsnetwork.cpp \
           pacsparameters.cpp \
           pacsserver.cpp \
           processimage.cpp \
           processimagesingleton.cpp \
           retrieveimages.cpp \
           dicomseries.cpp \
           starviewersettings.cpp \
           status.cpp \
           dicomstudy.cpp \
           multiplequerystudy.cpp \
           qquerystudythread.cpp \
           operation.cpp \
           qexecuteoperationthread.cpp \
           converttodicomdir.cpp \
           convertdicomtolittleendian.cpp \
           createdicomdir.cpp \
           dicomdirreader.cpp \
           storeimages.cpp \
           starviewerprocessimagestored.cpp \
           starviewerprocessimageretrieved.cpp \
           querypacs.cpp \
           dicommask.cpp \
           dicomdirimporter.cpp \
           qconfigurationscreen.cpp \
           qpacslist.cpp \
           qstudytreewidget.cpp \
           qserieslistwidget.cpp \
           qcreatedicomdir.cpp \
           qoperationstatescreen.cpp \
           queryscreen.cpp \
           qadvancedsearchwidget.cpp \
           qbasicsearchwidget.cpp \
           localdatabasemanager.cpp \
           localdatabaseimagedal.cpp \
           testdicomobjects.cpp \
           localdatabaseseriesdal.cpp \
           localdatabasestudydal.cpp \
           localdatabasepatientdal.cpp \
           localdatabaseutildal.cpp \
           testdatabase.cpp \
           qdeleteoldstudiesthread.cpp \
           localdatabasemanagerthreaded.cpp \
           databaseinstallation.cpp \
           qlocaldatabaseconfigurationscreen.cpp \
           parsexmlrispierrequest.cpp \
           listenrisrequest.cpp \
           processrisrequestthread.cpp \
           qlistenrisrequestsconfigurationscreen.cpp

INCLUDEPATH += ../core
DEPENDPATH += ../core

TEMPLATE = lib

DESTDIR = ./

include(../corelibsconfiguration.inc)
include(../vtk.inc)
include(../itk.inc)
include(../dcmtk.inc)
include(../log4cxx.inc)
include(../compilationtype.inc)

QT += xml network

