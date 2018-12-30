#ifndef CRE_FILTER_DIALOG_H
#define CRE_FILTER_DIALOG_H

#include <QDialog>

class QTextEdit;
class QLineEdit;
class QListWidget;

#include "CREFilterDefinitionManager.h"

class CREFilterDialog : public QDialog
{
    Q_OBJECT

    public:
        CREFilterDialog();

    protected:
        QListWidget* myList;
        QLineEdit* myName;
        QTextEdit* myScript;
        CREFilterDefinitionManager myFilters;
        int myFilterIndex;

        virtual void accept();
        virtual void reject();

        void refreshList();
        void saveCurrentFilter();

    protected slots:
        void onHelp();
        void onAdd();
        void onDelete();
        void currentRowChanged(int currentRow);
};

#endif // CRE_FILTER_DIALOG_H
