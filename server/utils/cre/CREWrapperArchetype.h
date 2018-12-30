#ifndef CRE_WRAPPER_ARCHETYPE_H
#define CRE_WRAPPER_ARCHETYPE_H

#include <QObject>

extern "C"
{
#include "global.h"
}

class CREWrapperObject;

class CREWrapperArchetype : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QObject* clone READ clone)

    public:
        CREWrapperArchetype(CREWrapperObject* parent, const archetype* arch);

        void setArchetype(const archetype* arch);

        QString name() const;
        QObject* clone();

    protected:
        const archetype* myArchetype;
        CREWrapperObject* myObject;
};

#endif // CRE_WRAPPER_ARCHETYPE_H
