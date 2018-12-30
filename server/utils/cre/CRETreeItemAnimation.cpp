extern "C" {
#include "global.h"
#include "image.h"
}

#include "CREAnimationPanel.h"
#include "CRETreeItemAnimation.h"

CRETreeItemAnimation::CRETreeItemAnimation(const Animations* animation)
{
    myAnimation = animation;
}

CRETreeItemAnimation::~CRETreeItemAnimation()
{
}

void CRETreeItemAnimation::fillPanel(QWidget* panel)
{
    CREAnimationPanel* p = static_cast<CREAnimationPanel*>(panel);
    p->setAnimation(myAnimation);
}
