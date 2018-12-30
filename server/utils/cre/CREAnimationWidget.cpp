#include <Qt>

#include "CREAnimationWidget.h"
#include "CREPixmap.h"

CREAnimationWidget::CREAnimationWidget(QWidget* parent) : QWidget(parent)
{
    myStep = 0;
    setMinimumWidth(32);
    setMinimumHeight(32);
}

void CREAnimationWidget::setAnimation(QList<int> faces)
{
    myFaces = faces;
    myStep = 0;
}

void CREAnimationWidget::step()
{
    if (myFaces.size() == 0)
        return;

    myStep++;
    if (myStep == myFaces.size())
        myStep = 0;

    repaint();
}

void CREAnimationWidget::paintEvent(QPaintEvent* /*event*/)
{
    if (myStep >= myFaces.size())
        return;

    QPainter painter(this);
    CREPixmap::getIcon(myFaces[myStep]).paint(&painter, 0, 0, 32, 32);
}
