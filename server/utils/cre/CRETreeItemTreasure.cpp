extern "C" {
#include "global.h"
#include "treasure.h"
}

#include "CRETreasurePanel.h"
#include "CRETreeItemTreasure.h"

CRETreeItemTreasure::CRETreeItemTreasure(const treasurelist* treasure)
{
    myTreasure = treasure;
}

CRETreeItemTreasure::~CRETreeItemTreasure()
{
}

void CRETreeItemTreasure::fillPanel(QWidget* panel)
{
    CRETreasurePanel* p = static_cast<CRETreasurePanel*>(panel);
    p->setTreasure(myTreasure);
}
