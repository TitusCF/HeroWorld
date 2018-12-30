#include <QtGui>

extern "C" {
#include "global.h"
#include "artifact.h"
}

#include "CREArtifactPanel.h"
#include "CREUtils.h"

CREArtifactPanel::CREArtifactPanel()
{
    myArtifact = NULL;

    QGridLayout* layout = new QGridLayout(this);

    QLabel* label = new QLabel(this);
    label->setText("Name:");
    layout->addWidget(label, 1, 1);
    myName = new QLineEdit(this);
    layout->addWidget(myName, 1, 2);

    label = new QLabel(this);
    label->setText("Chance:");
    layout->addWidget(label, 2, 1);
    myChance = new QLineEdit(this);
    layout->addWidget(myChance, 2, 2);

    label = new QLabel(this);
    label->setText("Type:");
    layout->addWidget(label, 3, 1);
    myType = new QLineEdit(this);
    layout->addWidget(myType, 3, 2);

    myViaAlchemy = new QLabel(this);
    myViaAlchemy->setWordWrap(true);
    layout->addWidget(myViaAlchemy, 4, 1, 1, 2);

    myArchetypes = new QTreeWidget(this);
    layout->addWidget(myArchetypes, 5, 1, 1, 2);
    myArchetypes->setHeaderLabel("Allowed/forbidden archetypes");
    myArchetypes->setIconSize(QSize(32, 32));
    myArchetypes->setRootIsDecorated(false);

    layout->addWidget(new QLabel(tr("Values:"), this), 6, 1, 1, 2);
    myValues = new QTextEdit(this);
    layout->addWidget(myValues, 7, 1, 1, 2);

    layout->addWidget(new QLabel(tr("Display result item for:"), this), 8, 1);
    myDisplay = new QComboBox(this);
    layout->addWidget(myDisplay, 8, 2);
    myInstance = new QTextEdit(this);
    layout->addWidget(myInstance, 9, 1, 1, 2);
    connect(myDisplay, SIGNAL(currentIndexChanged(int)), this, SLOT(displayArchetypeChanged(int)));
}

void CREArtifactPanel::computeMadeViaAlchemy(const artifact* artifact) const
{
    Q_ASSERT(artifact != NULL);

    const recipelist* list;
    const recipe* recipe;
    const archetype* arch;
    QStringList possible;

    for (int ing = 1; ; ing++)
    {
        list = get_formulalist(ing);
        if (!list)
            break;
        for (recipe = list->items; recipe; recipe = recipe->next)
        {
            if (recipe->title == NULL)
                continue;

            if (strcmp(recipe->title, artifact->item->name) != 0)
                continue;

            for (size_t a = 0; a < recipe->arch_names; a++)
            {
                arch = find_archetype(recipe->arch_name[a]);
                if (!arch)
                    continue;
                if ((arch->clone.type == artifact->item->type) && legal_artifact_combination(&arch->clone, artifact))
                {
                    if (!possible.contains(arch->name))
                        possible.append(arch->name);
                }
            }
        }
    }

    if (possible.isEmpty())
        myViaAlchemy->setText(tr("Can't be made via alchemy."));
    else
    {
        if (possible.size() == artifact->allowed_size)
            myViaAlchemy->setText(tr("Can be made via alchemy."));
        else
        {
            possible.sort();
            myViaAlchemy->setText(tr("The following archetypes can be used via alchemy: %1").arg(possible.join(tr(", "))));
        }
    }
}

void CREArtifactPanel::setArtifact(const artifact* artifact)
{
    Q_ASSERT(artifact);
    myArtifact = artifact;

    myName->setText(artifact->item->name);
    myChance->setText(QString::number(artifact->chance));
    myType->setText(QString::number(artifact->item->type));

    computeMadeViaAlchemy(artifact);

    const archt* arch;
    const char* name;
    QTreeWidgetItem* item;
    bool check;

    myArchetypes->clear();

    for (const linked_char* allowed = artifact->allowed; allowed; allowed = allowed->next)
    {
        name = allowed->name;
        if (name[0] == '!')
        {
            name = name + 1;
            check = false;
        }
        else
            check = true;

        arch = try_find_archetype(name);
        if (!arch)
            arch = find_archetype_by_object_name(name);

        if (arch)
        {
            item = CREUtils::archetypeNode(arch, NULL);
            item->setCheckState(0, check ? Qt::Checked : Qt::Unchecked);
            myArchetypes->addTopLevelItem(item);
        }
    }

    StringBuffer* dump = stringbuffer_new();
    get_ob_diff(dump, myArtifact->item, &empty_archetype->clone);
    char* final = stringbuffer_finish(dump);
    myValues->setText(final);
    free(final);

    myDisplay->clear();
    for (arch = first_archetype; arch != NULL; arch = arch->next)
    {
        if (arch->clone.type == artifact->item->type && legal_artifact_combination(&arch->clone, artifact))
        {
            myDisplay->addItem(tr("%1 [%2]").arg(arch->clone.name, arch->name));
            myDisplay->setItemData(myDisplay->count() - 1, arch->name, Qt::UserRole);
        }
    }
    if (myDisplay->count() > 0)
        displayArchetypeChanged(0);
    else
        myInstance->clear();
}

void CREArtifactPanel::displayArchetypeChanged(int index)
{
    if (index < 0 || index >= myDisplay->count())
        return;

    QByteArray name = myDisplay->itemData(index, Qt::UserRole).toString().toLatin1();
    if (name.isEmpty())
        return;
    archetype* arch = find_archetype(name);
    if (arch == NULL)
        return;

    char* desc;
    object* obj = arch_to_object(arch);
    SET_FLAG(obj, FLAG_IDENTIFIED);
    give_artifact_abilities(obj, myArtifact->item);
    desc = stringbuffer_finish(describe_item(obj, NULL, NULL));
    myInstance->setText(desc);
    free(desc);

    object_free2(obj, FREE_OBJ_FREE_INVENTORY | FREE_OBJ_NO_DESTROY_CALLBACK);
}
