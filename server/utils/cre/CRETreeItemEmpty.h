#ifndef _CRETREEITEMEMPTY_H
#define	_CRETREEITEMEMPTY_H

#include <QObject>
#include "CRETreeItem.h"

class CRETreeItemEmpty : public CRETreeItem
{
    Q_OBJECT

    public:
        CRETreeItemEmpty() { };
        virtual ~CRETreeItemEmpty() { };

        virtual QString getPanelName() const { return "(dummy)"; }
        virtual void fillPanel(QWidget*) { }
};

#endif	/* _CRETREEITEMEMPTY_H */

