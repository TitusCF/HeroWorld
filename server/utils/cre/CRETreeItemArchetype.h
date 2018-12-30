#ifndef CLASS_CRE_TREEITEM_ARCHETYPE_H
#define CLASS_CRE_TREEITEM_ARCHETYPE_H

#include <QObject>

#include "CRETreeItem.h"

extern "C" {
#include "global.h"
}

class CRETreeItemArchetype : public CRETreeItem
{
    Q_OBJECT
    public:
        CRETreeItemArchetype(const archt* archetype);
        virtual ~CRETreeItemArchetype();

        virtual QString getPanelName() const  { return "Archetype"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        const archt* myArchetype;

};

#endif // CLASS_CRE_TREEITEM_ARCHETYPE_H
