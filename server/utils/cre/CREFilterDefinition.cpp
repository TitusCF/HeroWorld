#include "CREFilterDefinition.h"

CREFilterDefinition::CREFilterDefinition()
{
}

CREFilterDefinition::CREFilterDefinition(const CREFilterDefinition& other) : QObject()
{
    myName = other.name();
    myFilter = other.filter();
}

CREFilterDefinition::~CREFilterDefinition()
{
}

const QString& CREFilterDefinition::name() const
{
    return myName;
}

void CREFilterDefinition::setName(const QString& name)
{
    myName = name;
}

const QString& CREFilterDefinition::filter() const
{
    return myFilter;
}

void CREFilterDefinition::setFilter(const QString& filter)
{
    myFilter = filter;
}

QDataStream &operator<<(QDataStream &out, const CREFilterDefinition &filter)
{
    out << filter.name() << filter.filter();
    return out;
}

QDataStream &operator>>(QDataStream &in, CREFilterDefinition &filter)
{
    QString data;
    in >> data;
    filter.setName(data);
    in >> data;
    filter.setFilter(data);
    return in;
}
