#ifndef CRE_WRAPPER_OBJECT_h
#define CRE_WRAPPER_OBJECT_h

extern "C" {
#include "global.h"
}

#include <QObject>
#include "CREWrapperArchetype.h"

class CREWrapperObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString race READ race)
    Q_PROPERTY(int type READ type)
    Q_PROPERTY(int level READ level)
    Q_PROPERTY(bool isMonster READ isMonster)
    Q_PROPERTY(bool isAlive READ isAlive)
    Q_PROPERTY(qint64 experience READ experience)
    Q_PROPERTY(quint32 attacktype READ attacktype)
    Q_PROPERTY(qint8 ac READ ac)
    Q_PROPERTY(qint8 wc READ wc)
    Q_PROPERTY(QObject* arch READ arch)
    Q_PROPERTY(qint16 damage READ damage)
    Q_PROPERTY(qint16 hp READ hp)
    Q_PROPERTY(qint32 weight READ weight)
    Q_PROPERTY(QString materialName READ materialName)

    public:
        CREWrapperObject();

        void setObject(const object* obj);

        CREWrapperArchetype* arch();
        QString name() const;
        QString race() const;
        int type() const;
        int level() const;
        bool isMonster() const;
        bool isAlive() const;
        qint64 experience() const;
        quint32 attacktype() const;
        qint8 ac() const;
        qint8 wc() const;
        qint16 damage() const;
        qint16 hp() const;
        qint32 weight() const;
        QString materialName() const;

    protected:
        const object* myObject;
        CREWrapperArchetype* myArchetype;
};

#endif // CRE_WRAPPER_OBJECT_h
