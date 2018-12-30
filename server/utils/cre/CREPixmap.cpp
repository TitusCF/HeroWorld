#include <Qt>
#include "CREPixmap.h"

extern "C" {
#include "global.h"
#include "face.h"
#include "image.h"
}

QHash<int, QIcon> CREPixmap::allFaces;
QIcon* CREPixmap::myTreasureIcon;
QIcon* CREPixmap::myTreasureOneIcon;
QIcon* CREPixmap::myTreasureYesIcon;
QIcon* CREPixmap::myTreasureNoIcon;
int CREPixmap::faceset;

void CREPixmap::init()
{
    myTreasureIcon = new QIcon(":resources/treasure_list.png");
    myTreasureOneIcon = new QIcon(":resources/treasureone_list.png");
    myTreasureYesIcon = new QIcon(":resources/treasure_yes.png");
    myTreasureNoIcon = new QIcon(":resources/treasure_no.png");

    faceset = 0;
}

QIcon CREPixmap::getIcon(int faceNumber)
{
    if (!allFaces.contains(faceNumber))
    {
        QPixmap face;

        if (facesets[faceset].faces[faceNumber].datalen > 0)
        {
            if (face.loadFromData((uchar*)facesets[faceset].faces[faceNumber].data, facesets[faceset].faces[faceNumber].datalen))
            {
                QIcon icon(face.scaled(32, 32, Qt::KeepAspectRatio));
                allFaces[faceNumber] = icon;
            }
        }
    }
    return allFaces[faceNumber];
}

QIcon CREPixmap::getTreasureIcon()
{
    Q_ASSERT(myTreasureIcon);
    return *myTreasureIcon;
}

QIcon CREPixmap::getTreasureOneIcon()
{
    Q_ASSERT(myTreasureOneIcon);
    return *myTreasureOneIcon;
}

QIcon CREPixmap::getTreasureYesIcon()
{
    Q_ASSERT(myTreasureYesIcon);
    return *myTreasureYesIcon;
}

QIcon CREPixmap::getTreasureNoIcon()
{
    Q_ASSERT(myTreasureNoIcon);
    return *myTreasureNoIcon;
}
