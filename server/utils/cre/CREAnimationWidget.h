#ifndef CREANIMATIONWIDGET_H_
#define CREANIMATIONWIDGET_H_

#include <QObject>
#include <QtGui>

class CREAnimationWidget : public QWidget
{
    Q_OBJECT

    public:
        CREAnimationWidget(QWidget* parent);

        void setAnimation(QList<int> faces);
        void step();
        //virtual QSize sizeHint () const { return QSize(32, 32); }

    protected:
        int myStep;
        QList<int> myFaces;

        virtual void paintEvent(QPaintEvent* event);
};

#endif // CREANIMATIONWIDGET_H_
