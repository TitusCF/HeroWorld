#include <Qt>
#include <QtGui>

extern "C" {
#include "global.h"
#include "recipe.h"
}

#include "CREFormulaePanel.h"
#include "CREUtils.h"

CREFormulaePanel::CREFormulaePanel()
{
    myRecipe = NULL;

    QGridLayout* layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("Title:"), this), 1, 1);
    myTitle = new QComboBox(this);

    layout->addWidget(myTitle, 1, 2);

    layout->addWidget(new QLabel(tr("Skill:"), this), 2, 1);
    mySkill = new QComboBox(this);
    layout->addWidget(mySkill, 2, 2);

    layout->addWidget(new QLabel(tr("Cauldron:"), this), 3, 1);
    myCauldron = new QComboBox(this);
    layout->addWidget(myCauldron, 3, 2);

    layout->addWidget(new QLabel(tr("Yield:"), this), 4, 1);
    myYield = new QLineEdit(this);
    layout->addWidget(myYield, 4, 2);

    layout->addWidget(new QLabel(tr("Chance:"), this), 5, 1);
    myChance = new QLineEdit(this);
    layout->addWidget(myChance, 5, 2);

    layout->addWidget(new QLabel(tr("Experience:"), this), 6, 1);
    myExperience = new QLineEdit(this);
    layout->addWidget(myExperience, 6, 2);

    layout->addWidget(new QLabel(tr("Difficulty:"), this), 7, 1);
    myDifficulty = new QLineEdit(this);
    layout->addWidget(myDifficulty, 7, 2);

    mySkill->addItem(tr("(none)"), 0);
    myCauldron->addItem(tr("(none)"), 0);
    const archt* arch = first_archetype;
    for (; arch; arch = arch->next)
    {
        if (arch->clone.type == SKILL)
            mySkill->addItem(arch->clone.name);
        if (QUERY_FLAG(&arch->clone, FLAG_IS_CAULDRON))
            myCauldron->addItem(arch->name);
    }

    layout->addWidget(new QLabel(tr("Index:"), this), 8, 1);
    myIndex = new QLabel(this);
    layout->addWidget(myIndex, 8, 2);

    myArchetypes = new QTreeWidget(this);
    myArchetypes->setHeaderLabel(tr("Archetypes:"));
    myArchetypes->setRootIsDecorated(false);
    myArchetypes->setIconSize(QSize(32, 32));
    layout->addWidget(myArchetypes, 9, 1, 1, 2);

    layout->addWidget(new QLabel(tr("Ingredients:"), this), 10, 1, 1, 2);
    myIngredients = new QTextEdit(this);
    layout->addWidget(myIngredients, 11, 1, 1, 2);

    QHBoxLayout* buttons = new QHBoxLayout;
    myValidate = new QPushButton(tr("&Validate"));
    buttons->addWidget(myValidate);
    myReset = new QPushButton(tr("&Reset"));
    buttons->addWidget(myReset);
    layout->addLayout(buttons, 12, 1, 1, 2);

    connect(myReset, SIGNAL(clicked(bool)), this, SLOT(resetClicked(bool)));
    connect(myValidate, SIGNAL(clicked(bool)), this, SLOT(validateClicked(bool)));
}

void CREFormulaePanel::setRecipe(const recipe* recipe)
{
    Q_ASSERT(recipe);
    myRecipe = recipe;

    myTitle->clear();
    myTitle->addItem("NONE");
    if (recipe->arch_names > 0)
    {
        archetype* arch = find_archetype(recipe->arch_name[0]);
        const artifactlist* at = find_artifactlist(arch->clone.type);
        if (at != NULL)
        {
            artifact* art = at->items;
            while (art)
            {
                if (art->item != NULL && art->item->name != NULL)
                    myTitle->addItem(art->item->name);
                art = art->next;
            }
        }
    }

    int index = myTitle->findText(recipe->title);
    if (index == -1)
        index = 0;
    myTitle->setCurrentIndex(index);
    myYield->setText(QString::number(recipe->yield));
    myChance->setText(QString::number(recipe->chance));
    myExperience->setText(QString::number(recipe->exp));
    myDifficulty->setText(QString::number(recipe->diff));
    myIndex->setText(QString::number(recipe->index));

    index = mySkill->findText(recipe->skill);
    if (index == -1)
        index = 0;
    mySkill->setCurrentIndex(index);

    index = myCauldron->findText(recipe->cauldron);
    if (index == -1)
        index = 0;
    myCauldron->setCurrentIndex(index);

    myArchetypes->clear();

    const archt* arch;
    for (size_t a = 0; a < recipe->arch_names; a++)
    {
        arch = find_archetype(recipe->arch_name[a]);
        myArchetypes->addTopLevelItem(CREUtils::archetypeNode(arch, NULL));
    }

    QStringList list;
    for (const linked_char* ing = myRecipe->ingred; ing; ing = ing->next)
    {
        list.append(ing->name);
    }
    myIngredients->setPlainText(list.join("\n"));
}

void CREFormulaePanel::resetClicked(bool)
{
    setRecipe(myRecipe);
}

void CREFormulaePanel::validateClicked(bool)
{
#if 0
    Q_ASSERT(myRecipe);

    myRecipe->setTitle(myTitle->text());
    myRecipe->setYield(myYield->text().toInt());
    myRecipe->setChance(myChance->text().toInt());
    myRecipe->setExperience(myExperience->text().toInt());
    myRecipe->setDifficulty(myDifficulty->text().toInt());
    if (mySkill->currentIndex() != 0)
        myRecipe->setSkill(mySkill->currentText());
    else
        myRecipe->setSkill("");
    if (myCauldron->currentIndex() != 0)
        myRecipe->setCauldron(myCauldron->currentText());
    else
        myRecipe->setCauldron("");

    QStringList arches;
    const Archetype* arch;
    ManagedReference ref;
    for (int a = 0; a < myArchetypes->topLevelItemCount(); a++)
    {
        ref = myArchetypes->topLevelItem(a)->data(0, Qt::UserRole).toInt();
        arch = DM_ARCHS->get(ref);
        arches.append(arch->name);
        DM_ARCHS->release(ref);
    }
    myRecipe->setArches(arches);

    myRecipe->setIngredients(myIngredients->toPlainText().split('\n'));
#endif
}
