#include "Quest.h"

QuestStep::QuestStep()
{
    myStep = 0;
    myIsCompletion = false;
}

int QuestStep::step() const
{
    return myStep;
}

void QuestStep::setStep(int step)
{
    myStep = step;
}

const QString& QuestStep::description() const
{
    return myDescription;
}

void QuestStep::setDescription(const QString& description)
{
    myDescription = description.trimmed();
}

bool QuestStep::isCompletion() const
{
    return myIsCompletion;
}

void QuestStep::setCompletion(bool completion)
{
    myIsCompletion = completion;
}

const QStringList& QuestStep::setWhen() const
{
    return mySetWhen;
}

QStringList& QuestStep::setWhen()
{
    return mySetWhen;
}


Quest::Quest()
{
    myCanRestart = false;
    myModified = false;
    myParent = NULL;
    myFaceNumber = 0;
}

Quest::~Quest()
{
    qDeleteAll(mySteps);
}

const QString& Quest::code() const
{
    return myCode;
}

void Quest::setCode(const QString& code)
{
    if (code == myCode)
        return;
    myCode = code;
    markModified();
}

const QString& Quest::title() const
{
    return myTitle;
}

void Quest::setTitle(const QString& title)
{
    if (title.trimmed() == myTitle)
        return;
    myTitle = title.trimmed();
    markModified();
}

const QString& Quest::face() const
{
    return myFace;
}

void Quest::setFace(const QString& face)
{
    if (myFace == face)
        return;
    myFace = face;
    markModified();
}

int Quest::faceNumber() const
{
  return myFaceNumber;
}

void Quest::setFaceNumber(int face)
{
  myFaceNumber = face;
}

const QString& Quest::description()const
{
    return myDescription;
}

void Quest::setDescription(const QString& description)
{
    if (description.trimmed() == myDescription)
        return;
    myDescription = description.trimmed();
    markModified();
}

bool Quest::canRestart() const
{
    return myCanRestart;
}

void Quest::setRestart(bool restart)
{
    if (myCanRestart == restart)
        return;
    myCanRestart = restart;
    markModified();
}

QList<const QuestStep*> Quest::steps() const
{
    QList<const QuestStep*> steps;
    foreach(const QuestStep* step, mySteps)
        steps.append(step);
    return steps;
}

QList<QuestStep*>& Quest::steps()
{
    return mySteps;
}

bool Quest::isModified() const
{
    return myModified;
}
void Quest::setModified(bool modified)
{
    myModified = modified;
}

void Quest::markModified()
{
    setModified(true);
    emit modified();
}

const Quest* Quest::parent() const
{
    return myParent;
}

void Quest::setParent(Quest* parent)
{
    if (parent != myParent)
    {
        myParent = parent;
        markModified();
    }
}

QList<CREMapInformation*>& Quest::maps()
{
    return myMaps;
}