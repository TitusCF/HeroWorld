#include <Qt>

extern "C" {
#include "global.h"
#include "face.h"
#include "image.h"
}

#include "CREFacePanel.h"
#include "CREUtils.h"
#include "CRESmoothFaceMaker.h"

/** @todo duplication with common/image */
static const char *const colorname[] = {
    "black",                    /* 0  */
    "white",                    /* 1  */
    "blue",                     /* 2  */
    "red",                      /* 3  */
    "orange",                   /* 4  */
    "light_blue",               /* 5  */
    "dark_orange",              /* 6  */
    "green",                    /* 7  */
    "light_green",              /* 8  */
    "grey",                     /* 9  */
    "brown",                    /* 10 */
    "yellow",                   /* 11 */
    "khaki"                     /* 12 */
};


CREFacePanel::CREFacePanel()
{
    myFace = 0;

    QGridLayout* layout = new QGridLayout(this);

    myUsing = new QTreeWidget(this);
    myUsing->setColumnCount(1);
    myUsing->setHeaderLabel(tr("Used by"));
    myUsing->setIconSize(QSize(32, 32));
    layout->addWidget(myUsing, 1, 1, 3, 2);

    myColor = new QComboBox(this);
    layout->addWidget(myColor, 4, 2);
    layout->addWidget(new QLabel("Magicmap color: "), 4, 1);

    for(uint color = 0; color < sizeof(colorname) / sizeof(*colorname); color++)
        myColor->addItem(colorname[color], color);

    myFile = new QLineEdit(this);
    myFile->setReadOnly(true);
    layout->addWidget(myFile, 5, 2);
    layout->addWidget(new QLabel("Original file: "), 5, 1);

    mySave = new QPushButton(tr("Save face"));
    layout->addWidget(mySave, 6, 1);
    connect(mySave, SIGNAL(clicked(bool)), this, SLOT(saveClicked(bool)));

    QPushButton* smooth = new QPushButton(tr("Make smooth base"), this);
    layout->addWidget(smooth, 6, 2);
    connect(smooth, SIGNAL(clicked(bool)), this, SLOT(makeSmooth(bool)));
}

void CREFacePanel::setFace(const New_Face* face)
{
    Q_ASSERT(face);
    myFace = face;

    myUsing->clear();

    QTreeWidgetItem* root = NULL;

    const archt* arch;
    sstring key;

    for (arch = first_archetype; arch; arch = arch->more ? arch->more : arch->next)
    {
      key = object_get_value(&arch->clone, "identified_face");
        if (arch->clone.face == myFace || (key && strcmp(face->name, key) == 0))
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

    const Animations* anim;

    // "bug" animation is zero, don't forget that shift
    for (int a = 0; a <= num_animations; a++)
    {
        anim = &animations[a];
        for (int face = 0; face < anim->num_animations; face++)
        {
            if (anim->faces[face] == myFace)
            {
                if (root == NULL)
                {
                    root = CREUtils::animationNode(NULL);
                    myUsing->addTopLevelItem(root);
                    root->setExpanded(true);
                }
                CREUtils::animationNode(anim, root);
                break;
            }
        }
    }

    root = NULL;

    const artifactlist* list;
    const artifact* arti;

    for (list = first_artifactlist; list; list = list->next)
    {
        for (arti = list->items; arti; arti = arti->next)
        {
            if (arti->item->face == myFace)
            {
                if (!root)
                {
                    root = CREUtils::artifactNode(NULL);
                    myUsing->addTopLevelItem(root);
                    root->setExpanded(true);
                }

                CREUtils::artifactNode(arti, root);
            }
        }
    }

    myColor->setCurrentIndex(myFace->magicmap);
}
void CREFacePanel::saveClicked(bool)
{
}

void CREFacePanel::makeSmooth(bool)
{
    CRESmoothFaceMaker maker;
    maker.setSelectedFace(myFace->number);
    maker.setAutoClose();
    maker.exec();
}
