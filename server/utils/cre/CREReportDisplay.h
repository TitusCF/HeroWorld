#ifndef CRE_REPORT_DISPLAY_H
#define CRE_REPORT_DISPLAY_H

#include <QObject>
#include <QDialog>

class CREReportDisplay : public QDialog
{
    Q_OBJECT

    public:
        CREReportDisplay(const QString& report);

    protected slots:
        void copyClicked(bool);
        void closeClicked(bool);

    protected:
        QString myReport;
};

#endif // CRE_REPORT_DISPLAY_H
