#include <QtGui>

#include "CREReportDialog.h"
#include "CRESettings.h"
#include "CREReportDefinition.h"

CREReportDialog::CREReportDialog()
{
    CRESettings settings;
    settings.loadReports(myReports);

    setWindowTitle(tr("Report parameters"));

    QGridLayout* layout = new QGridLayout(this);

    myList = new QListWidget(this);
    layout->addWidget(myList, 0, 0, 10, 2);

    QPushButton* add = new QPushButton(tr("Add"), this);
    connect(add, SIGNAL(clicked()), this, SLOT(onAdd()));
    layout->addWidget(add, 10, 0, 1, 1);

    QPushButton* del = new QPushButton(tr("Remove"), this);
    connect(del, SIGNAL(clicked()), this, SLOT(onDelete()));
    layout->addWidget(del, 10, 1, 1, 1);

    layout->addWidget(new QLabel(tr("Name:"), this), 0, 2, 1, 3);

    myName = new QLineEdit(this);
    layout->addWidget(myName, 1, 2, 1, 3);

    layout->addWidget(new QLabel(tr("Header:"), this), 2, 2, 1, 3);

    myHeader = new QTextEdit(this);
    layout->addWidget(myHeader, 3, 2, 1, 3);

    layout->addWidget(new QLabel(tr("Footer:"), this), 4, 2, 1, 3);

    myFooter = new QTextEdit(this);
    layout->addWidget(myFooter, 5, 2, 1, 3);

    layout->addWidget(new QLabel(tr("Item sort:"), this), 6, 2, 1, 3);

    mySort = new QTextEdit(this);
    layout->addWidget(mySort, 7, 2, 1, 3);

    layout->addWidget(new QLabel(tr("Item display:"), this), 8, 2, 1, 3);

    myDisplay = new QTextEdit(this);
    layout->addWidget(myDisplay, 9, 2, 1, 3);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close | QDialogButtonBox::Help, Qt::Horizontal, this);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttons, SIGNAL(helpRequested()), this, SLOT(onHelp()));

    layout->addWidget(buttons, 10, 2, 3, 1);

    setLayout(layout);
    connect(myList, SIGNAL(currentRowChanged(int)), this, SLOT(currentRowChanged(int)));
    refreshList();
}

void CREReportDialog::accept()
{
    saveCurrentReport();
    CRESettings settings;
    settings.saveReports(myReports);
    QDialog::accept();
}

void CREReportDialog::reject()
{
    if (QMessageBox::question(this, tr("Discard changes?"), tr("You are about to discard all changes!\nAre you sure?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    QDialog::reject();
}

void CREReportDialog::onHelp()
{
    QMessageBox::information(this, tr("Report help"), tr("Enter the script expression with which to Report items in the view. Current item is <b>item</b>, and it has the following properties:<br /><ul><li>for an archetype: name, clone</li><li>for a formulae: title, chance, difficulty, archs</li><li>for an artifact: item, chance, difficulty, allowed</li><li>for an object (for clone and item): type</li></ul><br />An item is shown if the expression evaluates to <i>true</i>.If a property is not defined for the current item, it will not be shown.<br /><br />Examples:<ul><li>items of type 5: <i>item.clone.type == 5</i></li><li>artifact allowed for all items of the type: <i>item.allowed.length == 0</i></il></ul>"));
}

void CREReportDialog::onAdd()
{
    saveCurrentReport();
    CREReportDefinition* report = new CREReportDefinition();
    report->setName(tr("<new Report>"));
    myReports.reports().append(report);
    refreshList();
    myList->setCurrentRow(myReports.reports().size() - 1);
}

void CREReportDialog::onDelete()
{
    if (myReportIndex == -1)
        return;

    CREReportDefinition* report = myReports.reports()[myReportIndex];
    if (QMessageBox::question(this, tr("Delete Report?"), tr("Really delete Report '%1'?").arg(report->name()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    delete report;
    myReports.reports().removeAt(myReportIndex);
    myReportIndex = -1;
    refreshList();
}

void CREReportDialog::refreshList()
{
    myList->clear();

    foreach(const CREReportDefinition* report, myReports.reports())
    {
        myList->addItem(report->name());
    }
    myReportIndex = -1;
}

void CREReportDialog::saveCurrentReport()
{
    if (myReportIndex != -1)
    {
        CREReportDefinition* report = myReports.reports()[myReportIndex];
        report->setName(myName->text());
        report->setHeader(myHeader->toPlainText());
        report->setItemDisplay(myDisplay->toPlainText());
        report->setItemSort(mySort->toPlainText());
        report->setFooter(myFooter->toPlainText());
        myList->item(myReportIndex)->setText(report->name());
    }
}

void CREReportDialog::currentRowChanged(int currentRow)
{
    saveCurrentReport();

    myReportIndex = -1;
    if (currentRow >= 0 && currentRow < myReports.reports().size())
    {
        const CREReportDefinition* report = myReports.reports()[currentRow];
        myName->setText(report->name());
        myHeader->setText(report->header());
        myFooter->setText(report->footer());
        mySort->setText(report->itemSort());
        myDisplay->setText(report->itemDisplay());
        myReportIndex = currentRow;
    }
}
