#include <Qt>
#include "CREMapInformation.h"
#include "CREMapInformationManager.h"

extern "C" {
#include "global.h"
}

#include "CREArchetypePanel.h"
#include "CREUtils.h"

CREArchetypePanel::CREArchetypePanel(CREMapInformationManager* store)
{
    Q_ASSERT(store);
    myStore = store;

    QGridLayout* layout = new QGridLayout(this);

    myDisplay = new QTextEdit(this);
    layout->addWidget(myDisplay, 1, 1);

    myUsing = new QTreeWidget(this);
    myUsing->setHeaderLabel(tr("Used by"));
    myUsing->setIconSize(QSize(32, 32));
    myUsing->setSortingEnabled(true);
    myUsing->sortByColumn(0, Qt::AscendingOrder);
    layout->addWidget(myUsing, 2, 1);
}

void CREArchetypePanel::setArchetype(const archt* archetype)
{
    myArchetype = archetype;
    StringBuffer* dump = stringbuffer_new();
    object_dump(&myArchetype->clone, dump);
    char* final = stringbuffer_finish(dump);
    myDisplay->setText(final);
    free(final);

    myUsing->clear();
    QTreeWidgetItem* root = NULL;

    const archt* arch;

    for (arch = first_archetype; arch; arch = (arch->more ? arch->more : arch->next))
    {
        if (arch->clone.other_arch == myArchetype)
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
            if (t->item == myArchetype)
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

    QList<CREMapInformation*> mapuse = myStore->getArchetypeUse(myArchetype);
    if (mapuse.size() > 0)
    {
        root = new QTreeWidgetItem(myUsing, QStringList(QTreeWidget::tr("Maps [%1]").arg(mapuse.size())));
        root->setExpanded(true);

        foreach(CREMapInformation* information, mapuse)
        {
            CREUtils::mapNode(information, root);
        }
    }
}
