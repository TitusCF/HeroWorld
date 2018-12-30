#ifndef CREANIMATIONCONTROL_H
#define CREANIMATIONCONTROL_H

#include <QObject>
#include <QtGui>

extern "C" {
#include "global.h"
#include "face.h"
}

class CREAnimationWidget;

class CREAnimationControl : public QWidget
{
    Q_OBJECT

    public:
        CREAnimationControl(QWidget* parent);
        void setAnimation(const Animations* animation);

    protected:
        const Animations* myAnimation;
        int myStep;
        int myLastStep;
        int myFacings;

        void display(const Animations* animation);
        QList<CREAnimationWidget*> myWidgets;

    private slots:
        void step();
};

#endif // CREANIMATIONCONTROL_H
