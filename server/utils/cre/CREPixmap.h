#ifndef CREPIXMAP_H
#define CREPIXMAP_H

#include <QtGui>

class CREPixmap
{
    public:
        static void init();

        static int faceset;
        static QIcon getIcon(int faceNumber);

        static QIcon getTreasureIcon();
        static QIcon getTreasureOneIcon();
        static QIcon getTreasureYesIcon();
        static QIcon getTreasureNoIcon();

    protected:
        static QHash<int, QIcon> allFaces;
        static QIcon* myTreasureIcon;
        static QIcon* myTreasureOneIcon;
        static QIcon* myTreasureYesIcon;
        static QIcon* myTreasureNoIcon;
};

#endif // CREPIXMAP_H
