#include <QtGui>
extern "C" {
#include "global.h"
}
#include "CREArchetypePanel.h"
#include "CRETreeItemArchetype.h"

CRETreeItemArchetype::CRETreeItemArchetype(const archt* archetype)
{
    myArchetype = archetype;
}

CRETreeItemArchetype::~CRETreeItemArchetype()
{
}

void CRETreeItemArchetype::fillPanel(QWidget* panel)
{
    CREArchetypePanel* p = static_cast<CREArchetypePanel*>(panel);
    p->setArchetype(myArchetype);
}
