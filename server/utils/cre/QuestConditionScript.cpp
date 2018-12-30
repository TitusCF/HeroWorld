#include "QuestConditionScript.h"

QuestConditionScript::QuestConditionScript(const QString& name, const QString& comment)
{
    myName = name;
    myComment = comment;
}

QuestConditionScript::~QuestConditionScript() {
}

const QString& QuestConditionScript::name() const
{
    return myName;
}

const QString& QuestConditionScript::comment() const
{
    return myComment;
}
