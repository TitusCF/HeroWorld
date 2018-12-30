#ifndef CRETREEITEMFORMULAE_H
#define CRETREEITEMFORMULAE_H

#include <QObject>
#include "CRETreeItem.h"

#include "global.h"

class CRETreeItemFormulae : public CRETreeItem
{
    Q_OBJECT

    public:
        CRETreeItemFormulae(const recipe* recipe);

        virtual QString getPanelName() const  { return "Formulae"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        const recipe* myRecipe;
};

#endif // CRETREEITEMFORMULAE_H
