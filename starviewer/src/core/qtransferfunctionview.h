#ifndef QTRANSFERFUNCTIONVIEW_H
#define QTRANSFERFUNCTIONVIEW_H

#include <QWidget>

namespace udg {

class ColorTransferFunction;
class TransferFunction;
class TransferFunctionEditor;

/**
    Classe base per a totes les vistes de funcions de transfer�ncia.
 */
class QTransferFunctionView : public QWidget {

    Q_OBJECT

public:

    /// Crea la vista, que treballar� amb l'editor donat.
    explicit QTransferFunctionView(TransferFunctionEditor *editor, QWidget *parent = 0);
    virtual ~QTransferFunctionView();

protected:

    /// Fa les connexions pertinents de signals i slots.
    virtual void makeConnections();

protected slots:

    /// Assigna la funci� de transfer�ncia.
    virtual void setTransferFunction(const TransferFunction &transferFunction) = 0;
    /// Assigna el nom de la funci� de transfer�ncia.
    virtual void setName(const QString &name) = 0;
    /// Assigna la funci� de transfer�ncia de color.
    virtual void setColorTransferFunction(const ColorTransferFunction &colorTransferFunction) = 0;
    /// Afegeix un punt de color.
    virtual void addColorPoint(double x, const QColor &color) = 0;
    /// Esborra un punt de color.
    virtual void removeColorPoint(double x) = 0;
    /// Mou un punt de color.
    virtual void moveColorPoint(double origin, double destination) = 0;
    /// Canvia el color d'un punt.
    virtual void changeColorPoint(double x, const QColor &color) = 0;

protected:

    /// L'editor.
    TransferFunctionEditor *m_editor;

private:

    Q_DISABLE_COPY(QTransferFunctionView)

};

} // namespace udg

#endif // QTRANSFERFUNCTIONVIEW_H
