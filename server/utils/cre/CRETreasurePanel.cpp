#include <Qt>

extern "C" {
#include "global.h"
#include "treasure.h"
}

#include "CRETreasurePanel.h"
#include "CREUtils.h"

CRETreasurePanel::CRETreasurePanel()
{
    QGridLayout* layout = new QGridLayout(this);

    myUsing = new QTreeWidget(this);
    myUsing->setColumnCount(1);
    myUsing->setHeaderLabel(tr("Used by"));
    myUsing->setIconSize(QSize(32, 32));
    layout->addWidget(myUsing, 1, 1);
}

void CRETreasurePanel::setTreasure(const treasurelist* treas)
{
    myUsing->clear();
    myTreasure = treas;

    const archt* arch;
    QTreeWidgetItem* root = NULL;

    QString name = myTreasure->name;

    for (arch = first_archetype; arch; arch = arch->more ? arch->more : arch->next)
    {
        if (arch->clone.randomitems && name == arch->clone.randomitems->name)
        {
            if (root == NULL)
            {
                root = CREUtils::archetypeNode(NULL);
                myUsing->addTopLevelItem(root);
                root->setExpanded(true);
            }
            CREUtils::archetypeNode(arch, root);
        }
    }

    root = NULL;

    const treasurelist* list;
    const treasure* t;

    for (list = first_treasurelist; list; list = list->next)
    {
        for (t = list->items; t; t = t->next)
        {
            if (t->name == name)
            {
                if (root == NULL)
                {
                    root = CREUtils::treasureNode(NULL);
                    myUsing->addTopLevelItem(root);
                    root->setExpanded(true);
                }
                CREUtils::treasureNode(list, root);
            }
        }
    }
}
