#ifndef CREETREEITEMANIMATION_H
#define CREETREEITEMANIMATION_H

#include "CRETreeItem.h"

extern "C" {
#include "global.h"
#include "image.h"
}

class CRETreeItemAnimation : public CRETreeItem
{
    Q_OBJECT
    public:
        CRETreeItemAnimation(const Animations* animation);
        virtual ~CRETreeItemAnimation();

        virtual QString getPanelName() const { return "Animation"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        const Animations* myAnimation;
};

#endif // CREETREEITEMANIMATION_H
