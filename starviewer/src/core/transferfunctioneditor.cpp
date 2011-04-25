#include "transferfunctioneditor.h"

#include "transferfunctioneditorcommands.h"

#include <QPair>
#include <QUndoStack>

namespace udg {

TransferFunctionEditor::TransferFunctionEditor(QObject *parent)
    : QObject(parent)
{
    m_undoStack = new QUndoStack(this);
}

const TransferFunction& TransferFunctionEditor::transferFunction() const
{
    return m_transferFunction;
}

void TransferFunctionEditor::setTransferFunction(const TransferFunction &transferFunction)
{
    m_undoStack->push(new SetTransferFunctionCommand(this, transferFunction));
}

void TransferFunctionEditor::setTransferFunctionCommand(const TransferFunction &transferFunction)
{
    m_transferFunction = transferFunction;
    emit transferFunctionChanged(transferFunction);
}

void TransferFunctionEditor::setName(const QString &name)
{
    m_undoStack->push(new SetNameCommand(this, name));
}

void TransferFunctionEditor::setNameCommand(const QString &name)
{
    m_transferFunction.setName(name);
    emit nameChanged(name);
}

void TransferFunctionEditor::setColorTransferFunction(const ColorTransferFunction &colorTransferFunction)
{
    m_undoStack->push(new SetColorTransferFunctionCommand(this, colorTransferFunction));
}

void TransferFunctionEditor::setColorTransferFunctionCommand(const ColorTransferFunction &colorTransferFunction)
{
    m_transferFunction.setColorTransferFunction(colorTransferFunction);
    emit colorTransferFunctionChanged(colorTransferFunction);
}

void TransferFunctionEditor::addColorPoint(double x, const QColor &color)
{
    if (m_transferFunction.isSetColor(x))
    {
        m_undoStack->push(new ChangeColorPointCommand(this, x, color));
    }
    else
    {
        m_undoStack->push(new AddColorPointCommand(this, x, color));
    }
}

void TransferFunctionEditor::addColorPointCommand(double x, const QColor &color)
{
    Q_ASSERT(!m_transferFunction.isSetColor(x));
    m_transferFunction.setColor(x, color);
    emit colorPointAdded(x, color);
}

void TransferFunctionEditor::removeColorPoint(double x)
{
    if (m_transferFunction.isSetColor(x))
    {
        m_undoStack->push(new RemoveColorPointCommand(this, x));
    }
}

void TransferFunctionEditor::removeColorPointCommand(double x)
{
    Q_ASSERT(m_transferFunction.isSetColor(x));
    m_transferFunction.unsetColor(x);
    emit colorPointRemoved(x);
}

void TransferFunctionEditor::moveColorPoint(double origin, double destination)
{
    if (m_transferFunction.isSetColor(origin))
    {
        if (m_transferFunction.isSetColor(destination))
        {
            m_undoStack->beginMacro(tr("Move color point from %1 to %2").arg(origin).arg(destination));
            m_undoStack->push(new ChangeColorPointCommand(this, destination, m_transferFunction.getColor(origin)));
            m_undoStack->push(new RemoveColorPointCommand(this, origin));
            m_undoStack->endMacro();
        }
        else
        {
            m_undoStack->push(new MoveColorPointCommand(this, origin, destination));
        }
    }
}

void TransferFunctionEditor::moveColorPointCommand(double origin, double destination)
{
    QColor color = m_transferFunction.getColor(origin);
    m_transferFunction.unsetColor(origin);
    m_transferFunction.setColor(destination, color);
    emit colorPointMoved(origin, destination);
}

// TODO: �s correcta la implementaci�?
void TransferFunctionEditor::moveColorPoints(const QList<double> &origins, double offset)
{
    if (origins.isEmpty())
    {
        return;
    }

    // Creem una llista d'accions a fer per cada origen i destinaci�

    enum Action { Add, Change, Remove };
    QMap< double, QPair<QColor, Action> > actions;
    int numberOfMoves = 0;

    foreach (double origin, origins)
    {
        if (!m_transferFunction.isSetColor(origin)) // l'origen no est� definit a la funci�, passem
        {
            continue;
        }

        double destination = origin + offset;

        if (actions.contains(origin))   // l'origen ja existeix a la llista, hauria de ser com a destinaci�, �s a dir, Add o Change
        {
            Q_ASSERT(actions[origin].second != Remove);
        }
        else    // l'origen no existeix a la llista, el marquem per esborrar
        {
            actions[origin] = qMakePair(QColor(), Remove);
        }

        if (actions.contains(destination))  // la destinaci� ja existeix a la llista, hauria de ser com a origen, �s a dir, Remove; la marquem per canviar
        {
            Q_ASSERT(actions[destination].second == Remove);
            actions[destination] = qMakePair(m_transferFunction.getColor(origin), Change);
        }
        else    // la destinaci� no existeix a la llista
        {
            if (m_transferFunction.isSetColor(destination)) // la destinaci� est� definida a la funci�, la marquem per canviar
            {
                actions[destination] = qMakePair(m_transferFunction.getColor(origin), Change);
            }
            else    // la destinaci� no est� definida a la funci�, la marquem per afegir
            {
                actions[destination] = qMakePair(m_transferFunction.getColor(origin), Add);
            }
        }

        numberOfMoves++;
    }

    // Executem les accions decidides

    QList<double> keys = actions.keys();
    m_undoStack->beginMacro(tr("Move %1 color points").arg(numberOfMoves));

    for (int i = 0; i < keys.size(); i++)
    {
        double x = keys.at(i);
        QColor color = actions[x].first;
        Action action = actions[x].second;

        switch (action)
        {
            case Add: m_undoStack->push(new AddColorPointCommand(this, x, color)); break;
            case Change: m_undoStack->push(new ChangeColorPointCommand(this, x, color)); break;
            case Remove: m_undoStack->push(new RemoveColorPointCommand(this, x)); break;
        }
    }

    m_undoStack->endMacro();
}

void TransferFunctionEditor::changeColorPoint(double x, const QColor &color)
{
    m_undoStack->push(new ChangeColorPointCommand(this, x, color));
}

void TransferFunctionEditor::changeColorPointCommand(double x, const QColor &color)
{
    Q_ASSERT(m_transferFunction.isSetColor(x));
    m_transferFunction.setColor(x, color);
    emit colorPointChanged(x, color);
}

void TransferFunctionEditor::redo()
{
    m_undoStack->redo();
}

void TransferFunctionEditor::undo()
{
    m_undoStack->undo();
}

} // namespace udg
