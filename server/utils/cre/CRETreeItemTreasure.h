#ifndef CRETREEITEMTREASURE_H
#define CRETREEITEMTREASURE_H

#include <QObject>

#include "CRETreeItem.h"

extern "C" {
#include "global.h"
}

class CRETreeItemTreasure : public CRETreeItem
{
    Q_OBJECT
    public:
        CRETreeItemTreasure(const treasurelist* treasure);
        virtual ~CRETreeItemTreasure();

        virtual QString getPanelName() const  { return "Treasure"; }
        virtual void fillPanel(QWidget* panel);

    protected:
        const treasurelist* myTreasure;

};

#endif // CRETREEITEMTREASURE_H
