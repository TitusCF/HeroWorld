extern "C" {
#include "global.h"
#include "face.h"
}

#include "CREFacePanel.h"
#include "CRETreeItemFace.h"

CRETreeItemFace::CRETreeItemFace(const New_Face* face)
{
    myFace = face;
}

CRETreeItemFace::~CRETreeItemFace()
{
}

void CRETreeItemFace::fillPanel(QWidget* panel)
{
    Q_ASSERT(myFace);
    CREFacePanel* p = static_cast<CREFacePanel*>(panel);
    p->setFace(myFace);
}
