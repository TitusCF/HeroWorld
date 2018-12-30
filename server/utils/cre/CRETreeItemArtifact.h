#ifndef CRETREEITEMARTIFACT_H
#define CRETREEITEMARTIFACT_H

#include <QObject>

#include "CRETreeItem.h"

extern "C" {
#include "global.h"
}

class CRETreeItemArtifact : public CRETreeItem
{
    Q_OBJECT

    public:
        CRETreeItemArtifact(const artifact* artifact);

        virtual QString getPanelName() const  { return "Artifact"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        const artifact* myArtifact;
};

#endif // CRETREEITEMARTIFACT_H
