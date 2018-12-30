#ifndef _CRETREEITEMQUEST_H
#define	_CRETREEITEMQUEST_H

#include <QObject>
#include "CRETreeItem.h"

class Quest;
class QTreeWidgetItem;
class CREResourcesWindow;

class CRETreeItemQuest : public CRETreeItem
{
    Q_OBJECT

    public:
        CRETreeItemQuest(Quest* quest, QTreeWidgetItem* item, CREResourcesWindow* window);
        virtual ~CRETreeItemQuest();
        virtual QString getPanelName() const;
        virtual void fillPanel(QWidget* panel);
        virtual void fillContextMenu(QMenu* menu);

    protected:
        Quest* myQuest;
        QTreeWidgetItem* myItem;
        CREResourcesWindow* myWindow;

    protected slots:
        void questModified();
        void deleteQuest(bool);
};

#endif	/* _CRETREEITEMQUEST_H */
