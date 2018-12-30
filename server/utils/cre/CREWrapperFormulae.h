#ifndef CRE_WRAPPER_FORMULAE_H
#define CRE_WRAPPER_FORMULAE_H

#include <QObject>
#include <QStringList>

extern "C" {
#include "global.h"
}

#include "CREWrapperObject.h"

class CREWrapperFormulae : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fullname READ fullname)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(int chance READ chance)
    Q_PROPERTY(int difficulty READ difficulty)
    Q_PROPERTY(int experience READ experience)
    Q_PROPERTY(QStringList archs READ archs)
    Q_PROPERTY(QStringList ingredients READ ingredients)

    public:
        CREWrapperFormulae();

        void setFormulae(const recipe* rec);

        QString fullname() const;
        QString title() const;
        int chance() const;
        int difficulty() const;
        int experience() const;
        QStringList archs() const;
        QStringList ingredients() const;

    protected:
        const recipe* myFormulae;
};

#endif // CRE_WRAPPER_FORMULAE_H
