#include "CRETreeItemRegion.h"
#include "CRERegionPanel.h"

CRETreeItemRegion::CRETreeItemRegion(regiondef* region)
{
    myRegion = region;
}

CRETreeItemRegion::~CRETreeItemRegion()
{
}

void CRETreeItemRegion::fillPanel(QWidget* panel)
{
    Q_ASSERT(myRegion);
    CRERegionPanel* p = static_cast<CRERegionPanel*>(panel);
    p->setRegion(myRegion);
}
