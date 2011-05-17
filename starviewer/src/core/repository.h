#ifndef UDGREPOSITORY_H
#define UDGREPOSITORY_H

#include "repositorybase.h"
#include "identifier.h"
#include <QHash>

namespace udg {

template< typename ItemType >
class Repository : public RepositoryBase {
public:

    /// Constructor per defecte
    Repository() : RepositoryBase()
    {
        m_nextIdentifierNumber = 0;
    }

    /// El destructor allibera l'espai ocupat pels items, si és que en queden i no s'han alliberat
    ~Repository();

    /**
        Afegeix un item al repositori.
        Ens retorna l'id de l'item afegit per poder-lo obtenir més endavant.
    */
    Identifier addItem(ItemType *item);

    /// Ens retorna un item del repositori amb l'identificador que especifiquem.
    ItemType *getItem(const Identifier id);

    /// Ens retorna tots els items del repositoy.
    QList<ItemType*> getItems();

    /// Elimina un item del repositori
    void removeItem(const Identifier id);

    /// Retorna el nombre d'items que hi ha al repositori
    int getNumberOfItems();

    /// Retorna una llista amb tots els id del repositori
    QList<Identifier> getIdentifiers();

    /// Elimina tots els elements que hi hagi al repositori
    void cleanUp();

private:

    typedef QHash<Identifier, ItemType*> ItemListType;
    typedef typename ItemListType::const_iterator ItemListIteratorType;

    /// La llista que conté els items amb un ID
    ItemListType m_itemList;

    /// Comptador d'ids. S'incrementa per cada nou item afegit
    int m_nextIdentifierNumber;
};

};  // End namespace udg

#include "repository.cpp"

#endif
