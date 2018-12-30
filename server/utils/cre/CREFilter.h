#ifndef CRE_FILTER_H
#define CRE_FILTER_H

class QObject;
#include <QString>
#include "CREScriptEngine.h"

class CREFilter
{
    public:
        CREFilter();

        bool showItem(QObject* item);

        const QString& filter() const;
        void setFilter(const QString& filter);

    protected:
        QString myFilter;
        CREScriptEngine myEngine;
};

#endif // CRE_FILTER_H
