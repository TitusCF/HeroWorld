#ifndef _CRETREEITEMMAP_H
#define	_CRETREEITEMMAP_H

class CREMapInformation;

#include "CRETreeItem.h"

class CRETreeItemMap : public CRETreeItem
{
    public:
        CRETreeItemMap(CREMapInformation* map);
        virtual ~CRETreeItemMap();

        virtual QString getPanelName() const  { return "Map"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        CREMapInformation* myMap;
};

#endif	/* _CRETREEITEMMAP_H */

