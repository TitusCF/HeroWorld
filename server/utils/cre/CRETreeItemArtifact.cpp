#include <Qt>

extern "C" {
#include "global.h"
#include "artifact.h"
}

#include "CREArtifactPanel.h"
#include "CRETreeItemArtifact.h"
#include "CREUtils.h"

CRETreeItemArtifact::CRETreeItemArtifact(const artifact* artifact)
{
    myArtifact = artifact;
}

void CRETreeItemArtifact::fillPanel(QWidget* panel)
{
    CREArtifactPanel* p = static_cast<CREArtifactPanel*>(panel);
    p->setArtifact(myArtifact);
}
