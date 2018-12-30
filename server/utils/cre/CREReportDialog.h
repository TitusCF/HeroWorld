#ifndef CRE_REPORT_DIALOG_H
#define CRE_REPORT_DIALOG_H

#include <QDialog>

class QTextEdit;
class QLineEdit;
class QListWidget;

#include "CREReportDefinitionManager.h"

class CREReportDialog : public QDialog
{
    Q_OBJECT

    public:
        CREReportDialog();

    protected:
        QListWidget* myList;
        QLineEdit* myName;
        QTextEdit* myHeader;
        QTextEdit* mySort;
        QTextEdit* myDisplay;
        QTextEdit* myFooter;
        CREReportDefinitionManager myReports;
        int myReportIndex;

        virtual void accept();
        virtual void reject();

        void refreshList();
        void saveCurrentReport();

    protected slots:
        void onHelp();
        void onAdd();
        void onDelete();
        void currentRowChanged(int currentRow);
};

#endif // CRE_REPORT_DIALOG_H
