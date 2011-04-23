#ifndef QTRANSFERFUNCTIONEDITOR2_H
#define QTRANSFERFUNCTIONEDITOR2_H

#include "ui_qtransferfunctioneditor2base.h"

namespace udg {

class QTransferFunctionView;
class TransferFunction;
class TransferFunctionEditor;

/**
    Nou widget d'edici� de funcions de transfer�ncia que ha de substituir els antics.
    Permet editar totalment una funci� de transfer�ncia: nom, color, opacitat scalar i opacitat del gradient.
    Permet desfer i refer els canvis, carregar i desar funcions, t� una llista de funcions recents, etc.
    T� una modalitat gr�fica i una de taula. Es pot crear amb nom�s un subconjunt de les caracter�stiques, si no es volen les altres.
    Per exemple, es pot tenir un editor nom�s gr�fic i nom�s per opacitats scalars, sense res m�s.
 */
class QTransferFunctionEditor2 : public QWidget, private ::Ui::QTransferFunctionEditor2Base {

    Q_OBJECT
    Q_ENUMS(View)

public:

    /// Caracter�stiques de l'editor.
    enum Feature {
        Name = 0x01,
        Color = 0x02,
        ScalarOpacity = 0x04,
        GradientOpacity = 0x08,
        LoadSave = 0x10,
        Recent = 0x20,
        Everything = 0x3f   // 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20
    };
    Q_DECLARE_FLAGS(Features, Feature)

    /// Tipus de vistes de l'editor.
    enum View { Graphical, Table, Both };

public:

    /// Crea l'editor amb totes les caracter�stiques i tots els tipus de vistes.
    explicit QTransferFunctionEditor2(QWidget *parent = 0);
    /// Crea l'editor amb les caracter�stiques i les vistes passades per par�metre.
    explicit QTransferFunctionEditor2(Features features, View view, QWidget *parent = 0);

    /// Retorna la funci� de transfer�ncia representada actualment.
    const TransferFunction& transferFunction() const;
    /// Assigna la funci� de transfer�ncia.
    void setTransferFunction(const TransferFunction &transferFunction);

signals:

    /// S'emet quan canvia la funci� de transfer�ncia.
    void transferFunctionChanged(const TransferFunction &transferFunction);

private:

    Q_DISABLE_COPY(QTransferFunctionEditor2)

    /// Inicialitza l'editor amb els par�metres donats.
    void init(Features features, View view);
    /// Fa les connexions pertinents de signals i slots.
    void makeConnections();

private slots:

    /// Emet el signal transferFunctionChanged(TransferFunction).
    void emitTransferFunctionChanged();

private:

    /// L'aut�ntic editor.
    TransferFunctionEditor *m_editor;
    /// Vista de la funci� de transfer�ncia.
    QTransferFunctionView *m_view;

};

} // namespace udg

#endif // QTRANSFERFUNCTIONEDITOR2_H
