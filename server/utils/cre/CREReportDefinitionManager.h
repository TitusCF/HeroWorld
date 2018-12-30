#ifndef CRE_REPORT_DEFINITION_MANAGER_H
#define CRE_REPORT_DEFINITION_MANAGER_H

class CREReportDefinition;

#include <QObject>
#include <QMetaType>

class CREReportDefinitionManager : public QObject
{
    Q_OBJECT

    public:
        CREReportDefinitionManager();
        CREReportDefinitionManager(const CREReportDefinitionManager& other);
        virtual ~CREReportDefinitionManager();

        void copy(const CREReportDefinitionManager& other);

        QList<CREReportDefinition*>& reports();
        const QList<CREReportDefinition*>& reports() const;

    protected:
        QList<CREReportDefinition*> myReports;
};

Q_DECLARE_METATYPE(CREReportDefinitionManager);

QDataStream &operator<<(QDataStream &out, const CREReportDefinitionManager &manager);
QDataStream &operator>>(QDataStream &in, CREReportDefinitionManager &manager);

#endif // CRE_REPORT_DEFINITION_MANAGER_H
