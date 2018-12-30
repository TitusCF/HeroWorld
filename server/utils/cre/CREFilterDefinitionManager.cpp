#include "CREFilterDefinitionManager.h"
#include "CREFilterDefinition.h"

CREFilterDefinitionManager::CREFilterDefinitionManager()
{
}

CREFilterDefinitionManager::CREFilterDefinitionManager(const CREFilterDefinitionManager& other) : QObject()
{
    copy(other);
}

CREFilterDefinitionManager::~CREFilterDefinitionManager()
{
    qDeleteAll(myFilters);
}

void CREFilterDefinitionManager::copy(const CREFilterDefinitionManager& other)
{
    qDeleteAll(myFilters);
    myFilters.clear();
    for (int f = 0; f < other.filters().size(); f++)
    {
        myFilters.append(new CREFilterDefinition(*other.filters()[f]));
    }
}

QList<CREFilterDefinition*>& CREFilterDefinitionManager::filters()
{
    return myFilters;
}

const QList<CREFilterDefinition*>& CREFilterDefinitionManager::filters() const
{
    return myFilters;
}

QDataStream &operator<<(QDataStream &out, const CREFilterDefinitionManager &manager)
{
    out << manager.filters().size();
    for (int f = 0; f < manager.filters().size(); f++)
        out << (*manager.filters()[f]);
    return out;
}

QDataStream &operator>>(QDataStream &in, CREFilterDefinitionManager &manager)
{
    int size;
    in >> size;
    while (size > 0)
    {
        CREFilterDefinition* filter = new CREFilterDefinition();
        in >> (*filter);
        manager.filters().append(filter);
        size--;
    }
    return in;
}
