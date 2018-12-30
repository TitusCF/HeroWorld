#ifndef CLASS_CRE_TREEITEM_FACE_H
#define CLASS_CRE_TREEITEM_FACE_H

#include <QObject>

#include "CRETreeItem.h"

extern "C" {
#include "global.h"
}

class CRETreeItemFace : public CRETreeItem
{
    Q_OBJECT
    public:
        CRETreeItemFace(const New_Face* face);
        virtual ~CRETreeItemFace();

        virtual QString getPanelName() const  { return "Face"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        const New_Face* myFace;

};

#endif // CLASS_CRE_TREEITEM_FACE_H
