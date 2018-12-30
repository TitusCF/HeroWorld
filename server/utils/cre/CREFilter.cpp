#include "CREFilter.h"
#include <QDebug>

CREFilter::CREFilter()
{
}

bool CREFilter::showItem(QObject* item)
{
    if (myFilter.isEmpty())
        return true;

    QScriptValue engineValue = myEngine.newQObject(item);
    myEngine.globalObject().setProperty("item", engineValue);

    myEngine.pushContext();
    bool show = myEngine.evaluate(myFilter).toBoolean();
    myEngine.popContext();
    if (myEngine.hasUncaughtException())
    {
        //qDebug() << myEngine.uncaughtException().toString();
        return false;
    }

    return show;
}

const QString& CREFilter::filter() const
{
    return myFilter;
}

void CREFilter::setFilter(const QString& filter)
{
    myFilter = filter;
}
