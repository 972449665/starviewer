#ifndef Q2DVIEWEREXTENSIONMEDIATOR_H
#define Q2DVIEWEREXTENSIONMEDIATOR_H

#include "extensionmediator.h"

#include "installextension.h"
#include "q2dviewerextension.h"

namespace udg {

class Q2DViewerExtensionMediator : public ExtensionMediator {
Q_OBJECT
public:
    Q2DViewerExtensionMediator(QObject *parent = 0);
    ~Q2DViewerExtensionMediator();

    virtual bool initializeExtension(QWidget *extension, const ExtensionContext &extensionContext);

    virtual DisplayableID getExtensionID() const;

    /// Orders the extension to view newly loaded studies from the current patient.
    virtual void viewNewStudiesFromSamePatient(QWidget *extension);

};

static InstallExtension<Q2DViewerExtension, Q2DViewerExtensionMediator> registerQ2DViewerExtension;

} // End udg namespace

#endif
