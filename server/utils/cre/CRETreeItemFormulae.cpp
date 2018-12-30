#include <Qt>
#include <QtGui>

extern "C" {
#include "global.h"
#include "treasure.h"
}

#include "CREFormulaePanel.h"
#include "CRETreeItemFormulae.h"

CRETreeItemFormulae::CRETreeItemFormulae(const recipe* recipe)
{
    Q_ASSERT(recipe);
    myRecipe = recipe;
}

void CRETreeItemFormulae::fillPanel(QWidget* panel)
{
    CREFormulaePanel* p = static_cast<CREFormulaePanel*>(panel);
    p->setRecipe(myRecipe);
}
