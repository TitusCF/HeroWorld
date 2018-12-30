#ifndef CRE_FILTER_DEFINITION_MANAGER_H
#define CRE_FILTER_DEFINITION_MANAGER_H

class CREFilterDefinition;

#include <QObject>
#include <QMetaType>

class CREFilterDefinitionManager : public QObject
{
    Q_OBJECT

    public:
        CREFilterDefinitionManager();
        CREFilterDefinitionManager(const CREFilterDefinitionManager& other);
        virtual ~CREFilterDefinitionManager();

        void copy(const CREFilterDefinitionManager& other);

        QList<CREFilterDefinition*>& filters();
        const QList<CREFilterDefinition*>& filters() const;

    protected:
        QList<CREFilterDefinition*> myFilters;
};

Q_DECLARE_METATYPE(CREFilterDefinitionManager);

QDataStream &operator<<(QDataStream &out, const CREFilterDefinitionManager &manager);
QDataStream &operator>>(QDataStream &in, CREFilterDefinitionManager &manager);

#endif // CRE_FILTER_DEFINITION_MANAGER_H
