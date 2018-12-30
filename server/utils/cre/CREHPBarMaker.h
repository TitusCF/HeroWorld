#ifndef CREHPBARMAKER_H
#define	CREHPBARMAKER_H

#include <QDialog>
#include <QObject>

class QLineEdit;
class QSpinBox;

class CREHPBarMaker : public QDialog
{
    Q_OBJECT

    public:
        CREHPBarMaker();
        virtual ~CREHPBarMaker();

    protected:
        void adjustColor();

    protected slots:
        void makeBar();
        void browse(bool);
        void selectColor(bool);

    private:
        QLineEdit* myDestination;
        QLineEdit* myName;
        QSpinBox* myHeight;
        QSpinBox* myShift;
        QColor myColor;
        QPushButton *myColorSelect;
};

#endif	/* CREHPBARMAKER_H */

