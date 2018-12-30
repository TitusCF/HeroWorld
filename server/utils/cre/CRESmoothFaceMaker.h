#ifndef CRESMOOTHFACEMAKER_H
#define	CRESMOOTHFACEMAKER_H

#include <QDialog>
#include <QObject>

class QLineEdit;
class QComboBox;

class CRESmoothFaceMaker : public QDialog
{
    Q_OBJECT

    public:
        CRESmoothFaceMaker();
        virtual ~CRESmoothFaceMaker();

        int selectedFace() const;
        void setSelectedFace(int face);
        QString destination() const;

        void setAutoClose(bool autoClose = true);

    protected slots:
        void makeSmooth();
        void browse(bool);
        void destinationEdited(const QString&);

    private:
        bool myAutoClose;
        bool myOverwrite;
        QLineEdit* myDestination;
        QComboBox* myFace;
};

#endif	/* CRESMOOTHFACEMAKER_H */

