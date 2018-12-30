#include "CRETreeItemQuest.h"
#include "CREQuestPanel.h"
#include "Quest.h"
#include <QTreeWidgetItem>
#include "CREResourcesWindow.h"
#include "ResourcesManager.h"
#include "CREPixmap.h"

CRETreeItemQuest::CRETreeItemQuest(Quest* quest, QTreeWidgetItem* item, CREResourcesWindow* window)
{
    myQuest = quest;
    Q_ASSERT(item);
    myItem = item;
    Q_ASSERT(window);
    myWindow = window;

    if (myQuest != NULL)
        connect(myQuest, SIGNAL(modified()), this, SLOT(questModified()));
}

CRETreeItemQuest::~CRETreeItemQuest()
{
}

QString CRETreeItemQuest::getPanelName() const
{
    if (myQuest)
        return "Quest";
    return "(dummy)";
}

void CRETreeItemQuest::fillPanel(QWidget* panel)
{
    if (myQuest == NULL)
        return;

    Q_ASSERT(myQuest);
    CREQuestPanel* p = static_cast<CREQuestPanel*>(panel);
    p->setQuest(myQuest);
}

void CRETreeItemQuest::questModified()
{
    myItem->setText(0, myQuest->code());
    myItem->setIcon(0, QIcon());
    if (!myQuest->face().isEmpty())
    {
      const New_Face* face = myWindow->resourcesManager()->face(myQuest->face());
      if (face != NULL)
        myItem->setIcon(0, CREPixmap::getIcon(face->number));
    }
}

void CRETreeItemQuest::fillContextMenu(QMenu* menu)
{
    if (!myQuest)
        return;

    QAction* del = new QAction("delete quest", menu);
    connect(del, SIGNAL(triggered(bool)), this, SLOT(deleteQuest(bool)));
    menu->addAction(del);
}

void CRETreeItemQuest::deleteQuest(bool)
{
    Q_ASSERT(myQuest);
    myWindow->deleteQuest(myQuest);
}
