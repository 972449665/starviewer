#include "dicomtagreader.h"

#include "logging.h"
#include "dicomsequenceattribute.h"
#include "dicomattribute.h"
#include "dicomvalueattribute.h"
#include "dicomsequenceitem.h"
// Qt
#include <QStringList>
// Dcmtk
#include <dcfilefo.h>
#include <dcdeftag.h>
#include <dcmetinf.h>

namespace udg {

DICOMTagReader::DICOMTagReader() : m_dicomData(0), m_dicomHeader(0), m_hasValidFile(false)
{
}

DICOMTagReader::DICOMTagReader(const QString &filename, DcmDataset *dcmDataset) : m_dicomData(0), m_dicomHeader(0), m_hasValidFile(false)
{
    this->setDcmDataset(filename, dcmDataset);
}

DICOMTagReader::DICOMTagReader(const QString &filename) : m_dicomData(0), m_dicomHeader(0), m_hasValidFile(false)
{
    this->setFile(filename);
}

DICOMTagReader::~DICOMTagReader()
{
    deleteDataLastLoadedFile();
}

void DICOMTagReader::deleteDataLastLoadedFile()
{
    if (m_dicomData)
    {
        delete m_dicomData;
        m_dicomData = NULL;
    }
    if (m_dicomHeader)
    {
        delete m_dicomHeader;
        m_dicomHeader = NULL;
    }

    // Clear cache from previous file
    foreach (const DICOMTag &tag, m_sequencesCache.keys())
    {
        delete m_sequencesCache.take(tag);
    }
}

bool DICOMTagReader::setFile(const QString &filename)
{
    DcmFileFormat dicomFile;

    m_filename = filename;

    OFCondition status = dicomFile.loadFile(qPrintable(filename));
    if (status.good())
    {
        m_hasValidFile = true;
        // Eliminem l'objecte anterior si n'hi hagués
        deleteDataLastLoadedFile();

        m_dicomHeader = new DcmMetaInfo(*dicomFile.getMetaInfo());
        m_dicomData = dicomFile.getAndRemoveDataset();
    }
    else
    {
        m_hasValidFile = false;
        DEBUG_LOG(QString("Error en llegir l'arxiu [%1]\nPossible causa: %2 ").arg(filename).arg(status.text()));
        ERROR_LOG(QString("Error en llegir l'arxiu [%1]\nPossible causa: %2 ").arg(filename).arg(status.text()));
        return false;
    }

    return true;
}

bool DICOMTagReader::canReadFile() const
{
    return m_hasValidFile;
}

QString DICOMTagReader::getFileName() const
{
    return m_filename;
}

void DICOMTagReader::setDcmDataset(const QString &filename, DcmDataset *dcmDataset)
{
    if (!dcmDataset)
    {
        return;
    }

    m_filename = filename;
    // Assumim que sempre serà un fitxer vàlid.
    /// TODO Cal fer alguna validació en aquests casos?
    // Potser en aquests casos el filename hauria de ser sempre buit ja que així expressem
    // explícitament que llegim un element de memòria
    m_hasValidFile = true;

    deleteDataLastLoadedFile();

    m_dicomData = dcmDataset;
}

DcmDataset* DICOMTagReader::getDcmDataset() const
{
    return m_dicomData;
}

bool DICOMTagReader::tagExists(const DICOMTag &tag) const
{
    if (m_dicomData)
    {
        return m_dicomData->tagExists(DcmTagKey(tag.getGroup(), tag.getElement()));
    }
    else
    {
        DEBUG_LOG("No hi ha cap m_dicomData (DcmDataset) carregat");
        return false;
    }
}

QString DICOMTagReader::getValueAttributeAsQString(const DICOMTag &tag) const
{
    if (!m_dicomData)
    {
        DEBUG_LOG("No hi ha cap m_dicomData (DcmDataset) carregat. Tornem string buida.");
        return QString();
    }

    QString result;

    OFString value;
    OFCondition status = m_dicomData->findAndGetOFStringArray(DcmTagKey(tag.getGroup(), tag.getElement()), value);
    if (status.good())
    {
        result = value.c_str();
    }
    else
    {
        logStatusForTagOperation(tag, status);
    }

    return result;
}

DICOMValueAttribute* DICOMTagReader::getValueAttribute(const DICOMTag &attributeTag) const
{
    if (!m_dicomData)
    {
        DEBUG_LOG("No hi ha cap m_dicomData (DcmDataset) carregat. Tornem nul.");
        return 0;
    }
    
    DICOMValueAttribute *valueAttribute = 0;
    DcmElement *dicomElement = NULL;
    OFCondition status = m_dicomData->findAndGetElement(DcmTagKey(attributeTag.getGroup(), attributeTag.getElement()), dicomElement);
    if (status.good())
    {
        valueAttribute = convertToDICOMValueAttribute(dicomElement, DICOMTagReader::AllTags);
    }
    else
    {
        logStatusForTagOperation(attributeTag, status);
    }
    
    return valueAttribute;
}

DICOMSequenceAttribute* DICOMTagReader::getSequenceAttribute(const DICOMTag &sequenceTag, DICOMTagReader::ReturnValueOfTags returnValueOfTags) const
{
    if (!m_dicomData)
    {
        DEBUG_LOG("No m_dicomData (DcmDataset). Returning null.");
        return NULL;
    }

    // If sequence is cached, return it from cache
    if (m_sequencesCache.contains(sequenceTag))
    {
        return m_sequencesCache[sequenceTag];
    }

    // Get attributes of each item from a "first level" sequence
    DcmSequenceOfItems *sequence = NULL;
    OFCondition status = m_dicomData->findAndGetSequence(DcmTagKey(sequenceTag.getGroup(), sequenceTag.getElement()), sequence, OFTrue);

    if (status.good())
    {
        DEBUG_LOG(QString("Sequence %1 found").arg(sequenceTag.getKeyAsQString()));
        m_sequencesCache[sequenceTag] = convertToDICOMSequenceAttribute(sequence, returnValueOfTags);
        return m_sequencesCache[sequenceTag];
    }
    else
    {
        logStatusForTagOperation(sequenceTag, status);
        m_sequencesCache[sequenceTag] = NULL;
        return NULL;
    }
}

DICOMSequenceAttribute* DICOMTagReader::convertToDICOMSequenceAttribute(DcmSequenceOfItems *dcmtkSequence,
                                                                        DICOMTagReader::ReturnValueOfTags returnValueOfTags) const
{
    if (!dcmtkSequence)
    {
        return 0;
    }
    
    DICOMSequenceAttribute *sequenceAttribute = new DICOMSequenceAttribute();

    sequenceAttribute->setTag(DICOMTag(dcmtkSequence->getGTag(), dcmtkSequence->getETag()));

    for (unsigned int i = 0; i < dcmtkSequence->card(); i++)
    {
        DICOMSequenceItem *dicomItem = new DICOMSequenceItem();
        DcmItem *dcmtkItem = dcmtkSequence->getItem(i);
        for (unsigned int j = 0; j < dcmtkItem->card(); j++)
        {
            DcmElement *element = dcmtkItem->getElement(j);
            // És una Sequence of Items
            if (!element->isLeaf())
            {
                dicomItem->addAttribute(convertToDICOMSequenceAttribute(OFstatic_cast(DcmSequenceOfItems*, element), returnValueOfTags));
            }
            else
            {
                DICOMValueAttribute *dicomValueAttribute = convertToDICOMValueAttribute(element, returnValueOfTags);

                if (dicomValueAttribute != NULL)
                {
                    dicomItem->addAttribute(dicomValueAttribute);
                }
            }
        }
        sequenceAttribute->addItem(dicomItem);
    }

    return sequenceAttribute;
}

DICOMValueAttribute* DICOMTagReader::convertToDICOMValueAttribute(DcmElement *dcmtkDICOMElement, DICOMTagReader::ReturnValueOfTags returnValueOfTags) const
{
    if (!dcmtkDICOMElement)
    {
        return 0;
    }
    
    if (!dcmtkDICOMElement->isLeaf())
    {
        // Es tracta d'una seqüència, no es pot convertir a atribut
        return 0;
    }
    
    DICOMValueAttribute *dicomValueAttribute = new DICOMValueAttribute();
    dicomValueAttribute->setTag(DICOMTag(dcmtkDICOMElement->getGTag(), dcmtkDICOMElement->getETag()));

    if (returnValueOfTags != DICOMTagReader::ExcludeHeavyTags ||
        (dcmtkDICOMElement->getTag() != DcmTag(DCM_PixelData) && dcmtkDICOMElement->getTag() != DcmTag(DCM_OverlayData)))
    {
        OFString value;
        OFCondition status = dcmtkDICOMElement->getOFStringArray(value);

        if (status.good())
        {
            dicomValueAttribute->setValue(QString(value.c_str()));
        }
        else
        {
            dicomValueAttribute->setValue(QString("Unreadable tag value: %1").arg(status.text()));
            logStatusForTagOperation(DICOMTag(dcmtkDICOMElement->getTag().getGroup(), dcmtkDICOMElement->getTag().getElement()), status);            
        }
    }

    return dicomValueAttribute;
}

QList<DICOMAttribute*> DICOMTagReader::convertToDICOMAttributeQList(DcmItem *dcmItem, DICOMTagReader::ReturnValueOfTags returnValueOfTags) const
{
    if (!dcmItem)
    {
        return QList<DICOMAttribute*>();
    }
    
    QList<DICOMAttribute*> attributeList;
    DcmElement *currentElement = NULL;

    for (unsigned int i = 0; i < dcmItem->card(); i++)
    {
        currentElement = OFstatic_cast(DcmElement*, dcmItem->nextInContainer(currentElement));

        // Es tracta d'una seqüència
        if (!currentElement->isLeaf())
        {
            DICOMSequenceAttribute *dicomSequenceAttribute = convertToDICOMSequenceAttribute(OFstatic_cast(DcmSequenceOfItems*, currentElement),
                                                                                             returnValueOfTags);
            attributeList.append(dicomSequenceAttribute);
        }
        else
        {
            DICOMValueAttribute *dicomValueAttribute = convertToDICOMValueAttribute(currentElement, returnValueOfTags);

            if (dicomValueAttribute != NULL)
            {
                attributeList.append(dicomValueAttribute);
            }
        }
    }

    return attributeList;
}
QList<DICOMAttribute*> DICOMTagReader::getDICOMDataSet(DICOMTagReader::ReturnValueOfTags returnValueOfTags) const
{
    return convertToDICOMAttributeQList(m_dicomData, returnValueOfTags);
}

QList<DICOMAttribute*> DICOMTagReader::getDICOMHeader() const
{
    if (m_dicomHeader)
    {
        return convertToDICOMAttributeQList(m_dicomHeader, DICOMTagReader::AllTags);
    }
    else
    {
        return QList<DICOMAttribute*>();
    }
}

void DICOMTagReader::logStatusForTagOperation(const DICOMTag &tag, const OFCondition &status) const
{
    if (status.good())
    {
        return;
    }

    if (QString::compare(status.text(), "Tag Not Found", Qt::CaseInsensitive) != 0)
    {
        DEBUG_LOG(QString("S'ha produit el següent problema a l'intentar obtenir el tag %1 :: %2").arg(tag.getKeyAsQString()).arg(status.text()));
    }
}

}
