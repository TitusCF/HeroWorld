#include "CREWrapperObject.h"

CREWrapperObject::CREWrapperObject()
{
    myObject = NULL;
    myArchetype = NULL;
}

void CREWrapperObject::setObject(const object* obj)
{
    myObject = obj;
    if (myArchetype == NULL)
        myArchetype = new CREWrapperArchetype(this, obj->arch);
    else
        myArchetype->setArchetype(obj->arch);
}

CREWrapperArchetype* CREWrapperObject::arch()
{
    return myArchetype;
}

QString CREWrapperObject::name() const
{
    return myObject->name;
}

QString CREWrapperObject::race() const
{
    return myObject->race;
}

int CREWrapperObject::type() const
{
    return myObject->type;
}

int CREWrapperObject::level() const
{
    return myObject->level;
}

bool CREWrapperObject::isMonster() const
{
    return QUERY_FLAG(myObject, FLAG_MONSTER);
}

bool CREWrapperObject::isAlive() const
{
    return QUERY_FLAG(myObject, FLAG_ALIVE);
}

qint64 CREWrapperObject::experience() const
{
    return myObject->stats.exp;
}

quint32 CREWrapperObject::attacktype() const
{
    return myObject->attacktype;
}

qint8 CREWrapperObject::ac() const
{
    return myObject->stats.ac;
}

qint8 CREWrapperObject::wc() const
{
    return myObject->stats.wc;
}

qint16 CREWrapperObject::damage() const
{
    return myObject->stats.dam;
}

qint16 CREWrapperObject::hp() const
{
    return myObject->stats.hp;
}

qint32 CREWrapperObject::weight() const
{
  return myObject->weight;
}

QString CREWrapperObject::materialName() const
{
  return myObject->materialname;
}