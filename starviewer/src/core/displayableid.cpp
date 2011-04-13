#include "displayableid.h"

namespace udg {

DisplayableID::DisplayableID(const QString & id, const QString & name) : m_id(id),  m_name(name)
{
}

DisplayableID::~DisplayableID()
{
}


inline bool operator==(const DisplayableID &v1, const DisplayableID &v2)
{
     return v1.m_id == v2.m_id;
}

inline bool operator!=(const DisplayableID &v1, const DisplayableID &v2)
{
    return v1.m_id != v2.m_id;
}


inline bool operator<(const DisplayableID &v1, const DisplayableID &v2)
{
    return v1.m_id < v2.m_id;
}


inline bool operator>(const DisplayableID &v1, const DisplayableID &v2)
{
    return v1.m_id < v2.m_id;
}

}
