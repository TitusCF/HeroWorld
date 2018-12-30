#include "CRETreeItemMap.h"
#include "CREMapInformation.h"
#include "CREMapPanel.h"

CRETreeItemMap::CRETreeItemMap(CREMapInformation* map)
{
    myMap = map;
}

CRETreeItemMap::~CRETreeItemMap()
{
}

void CRETreeItemMap::fillPanel(QWidget* panel)
{
    Q_ASSERT(myMap);
    CREMapPanel* p = static_cast<CREMapPanel*>(panel);
    p->setMap(myMap);
}
