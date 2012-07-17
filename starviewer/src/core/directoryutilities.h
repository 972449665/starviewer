#ifndef UDGDIRECTORYUTILITIES_H
#define UDGDIRECTORYUTILITIES_H

#include <QObject>

class QString;
class QDir;

namespace udg {

/**
    Classe que encapsula operacions d'utilitat amb directoris
  */
class DirectoryUtilities : public QObject {
Q_OBJECT
public:
    DirectoryUtilities();
    ~DirectoryUtilities();

    /// Esborra el contingut del directori i el directori passat per par�metres
    /// @param directoryPath Path del directori a esborrar
    /// @param deleteRootDirectory Indica si s'ha d'esborrar nom�s el contingu del directori o tamb� el directori arrel passat per par�metre. Si fals nom�s
    /// s'esborra el contingut, si �s cert s'esborra el contingut i el directori passat per par�metre
    /// @return Indica si l'operacio s'ha realitzat amb �xit
    bool deleteDirectory(const QString &directoryPath, bool deleteRootDirectory);

    /// Copia el directori origen al directori dest�
    static bool copyDirectory(const QString &sourceDirectory, const QString &sourceDestination);
    
    /// Ens indica si un directori est� buit
    bool isDirectoryEmpty(const QString &directoryPath);

signals:
    void directoryDeleted();

private:
    bool removeDirectory(const QDir &dir, bool deleteRootDirectory);
};

}

#endif