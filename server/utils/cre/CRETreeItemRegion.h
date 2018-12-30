#ifndef _CRETREEITEMREGION_H
#define	_CRETREEITEMREGION_H

#include "CRETreeItem.h"
extern "C"
{
#include "global.h"
#include "map.h"
}

class CRETreeItemRegion : public CRETreeItem
{
    public:
        CRETreeItemRegion(regiondef* region);
        virtual ~CRETreeItemRegion();

        virtual QString getPanelName() const { return "Region"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        regiondef* myRegion;
};

#endif	/* _CRETREEITEMREGION_H */

