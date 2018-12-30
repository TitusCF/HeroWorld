#include "CREReportDefinitionManager.h"
#include "CREReportDefinition.h"

CREReportDefinitionManager::CREReportDefinitionManager()
{
}

CREReportDefinitionManager::CREReportDefinitionManager(const CREReportDefinitionManager& other) : QObject()
{
    copy(other);
}

CREReportDefinitionManager::~CREReportDefinitionManager()
{
    qDeleteAll(myReports);
}

void CREReportDefinitionManager::copy(const CREReportDefinitionManager& other)
{
    qDeleteAll(myReports);
    myReports.clear();
    for (int f = 0; f < other.reports().size(); f++)
    {
        Q_ASSERT(other.reports()[f]);
        myReports.append(new CREReportDefinition(*other.reports()[f]));
    }
}

QList<CREReportDefinition*>& CREReportDefinitionManager::reports()
{
    return myReports;
}

const QList<CREReportDefinition*>& CREReportDefinitionManager::reports() const
{
    return myReports;
}

QDataStream &operator<<(QDataStream &out, const CREReportDefinitionManager &manager)
{
    out << manager.reports().size();
    for (int f = 0; f < manager.reports().size(); f++)
        out << (*manager.reports()[f]);
    return out;
}

QDataStream &operator>>(QDataStream &in, CREReportDefinitionManager &manager)
{
    int size;
    in >> size;
    while (size > 0)
    {
        CREReportDefinition* report = new CREReportDefinition();
        in >> (*report);
        manager.reports().append(report);
        size--;
    }
    return in;
}
