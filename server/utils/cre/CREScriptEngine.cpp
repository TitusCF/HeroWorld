#include "CREScriptEngine.h"

extern "C"
{
#include "global.h"
#include "attack.h"
}

CREScriptEngine::CREScriptEngine()
{
    QScriptValue attacks = newObject();

    for (int attack = 0; attack < NROFATTACKS; attack++)
    {
        QString name(attacktype_desc[attack]);
        name = name.replace(' ', '_');
        attacks.setProperty(name, 1 << attack);
    }

    globalObject().setProperty("AttackType", attacks, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

CREScriptEngine::~CREScriptEngine()
{
}
