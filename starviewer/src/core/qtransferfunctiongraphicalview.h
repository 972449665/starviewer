#ifndef QTRANSFERFUNCTIONGRAPHICALVIEW_H
#define QTRANSFERFUNCTIONGRAPHICALVIEW_H

#include "qtransferfunctionview.h"
#include "ui_qtransferfunctiongraphicalviewbase.h"

namespace udg {

class TransferFunction;

/**
    Vista gr�fica d'una funci� de transfer�ncia.
 */
class QTransferFunctionGraphicalView : public QTransferFunctionView, private Ui::QTransferFunctionGraphicalViewBase {

    Q_OBJECT

public:

    /// Crea la vista, que treballar� amb l'editor donat.
    explicit QTransferFunctionGraphicalView(TransferFunctionEditor *editor, QWidget *parent = 0);

private:

    Q_DISABLE_COPY(QTransferFunctionGraphicalView)

    /// Fa les connexions pertinents de signals i slots.
    void makeConnections();
    /// Habilita les connexions de la vista cap a l'editor.
    void enableEditingConnections();
    /// Inhabilita les connexions de la vista cap a l'editor.
    void disableEditingConnections();

private slots:

    /// Assigna la funci� de transfer�ncia.
    void setTransferFunction(const TransferFunction &transferFunction);
    /// Assigna el nom de la funci� de transfer�ncia.
    void setName(const QString &name);
    /// Assigna la funci� de transfer�ncia de color.
    void setColorTransferFunction(const ColorTransferFunction &colorTransferFunction);
    //void moveColorPoints(const QList< QPair<double, double> > &moves);

private:

    /// Indica si les connexions de la vista cap a l'editor estan habilitades o no.
    bool m_editingConnectionsEnabled;

};

} // namespace udg

#endif // QTRANSFERFUNCTIONGRAPHICALVIEW_H
