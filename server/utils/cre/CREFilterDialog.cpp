#include <QtGui>

#include "CREFilterDialog.h"
#include "CREFilter.h"
#include "CRESettings.h"
#include "CREFilterDefinition.h"

CREFilterDialog::CREFilterDialog()
{
    CRESettings settings;
    settings.loadFilters(myFilters);

    setWindowTitle(tr("Filter parameters"));

    QGridLayout* layout = new QGridLayout(this);

    myList = new QListWidget(this);
    layout->addWidget(myList, 0, 0, 4, 2);

    QPushButton* add = new QPushButton(tr("Add"), this);
    connect(add, SIGNAL(clicked()), this, SLOT(onAdd()));
    layout->addWidget(add, 4, 0, 1, 1);

    QPushButton* del = new QPushButton(tr("Remove"), this);
    connect(del, SIGNAL(clicked()), this, SLOT(onDelete()));
    layout->addWidget(del, 4, 1, 1, 1);

    layout->addWidget(new QLabel(tr("Name:"), this), 0, 2, 1, 3);

    myName = new QLineEdit(this);
    layout->addWidget(myName, 1, 2, 1, 3);

    layout->addWidget(new QLabel(tr("Filter:"), this), 2, 2, 1, 3);

    myScript = new QTextEdit(this);
    layout->addWidget(myScript, 3, 2, 1, 3);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close | QDialogButtonBox::Help, Qt::Horizontal, this);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttons, SIGNAL(helpRequested()), this, SLOT(onHelp()));

    layout->addWidget(buttons, 4, 2, 3, 1);

    setLayout(layout);
    connect(myList, SIGNAL(currentRowChanged(int)), this, SLOT(currentRowChanged(int)));
    refreshList();
}

void CREFilterDialog::accept()
{
    saveCurrentFilter();
    CRESettings settings;
    settings.saveFilters(myFilters);
    QDialog::accept();
}

void CREFilterDialog::reject()
{
    if (QMessageBox::question(this, tr("Discard changes?"), tr("You are about to discard all changes!\nAre you sure?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    QDialog::reject();
}

void CREFilterDialog::onHelp()
{
    QMessageBox::information(this, tr("Filter help"), tr("Enter the script expression with which to filter items in the view. Current item is <b>item</b>, and it has the following properties:<br /><ul><li>for an archetype: name, race, type, level, isMonster, isAlive, experience, attacktype, ac, wc, arch (with a name property)</li><li>for a formulae: title, chance, difficulty, archs</li><li>for an artifact: item, chance, difficulty, allowed</li><li>for an object (for clone and item): type</li></ul><br />An item is shown if the expression evaluates to <i>true</i>.If a property is not defined for the current item, it will not be shown.<br /><br />Examples:<ul><li>items of type 5: <i>item.type == 5</i></li><li>artifact allowed for all items of the type: <i>item.allowed.length == 0</i></il></ul>"));
}

void CREFilterDialog::onAdd()
{
    saveCurrentFilter();
    CREFilterDefinition* filter = new CREFilterDefinition();
    filter->setName(tr("<new filter>"));
    myFilters.filters().append(filter);
    refreshList();
    myList->setCurrentRow(myFilters.filters().size() - 1);
}

void CREFilterDialog::onDelete()
{
    if (myFilterIndex == -1)
        return;

    CREFilterDefinition* filter = myFilters.filters()[myFilterIndex];
    if (QMessageBox::question(this, tr("Delete filter?"), tr("Really delete filter '%1'?").arg(filter->name()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    delete filter;
    myFilters.filters().removeAt(myFilterIndex);
    myFilterIndex = -1;
    refreshList();
}

void CREFilterDialog::refreshList()
{
    myList->clear();

    foreach(const CREFilterDefinition* filter, myFilters.filters())
    {
        myList->addItem(filter->name());
    }
    myFilterIndex = -1;
}

void CREFilterDialog::saveCurrentFilter()
{
    if (myFilterIndex != -1)
    {
        CREFilterDefinition* filter = myFilters.filters()[myFilterIndex];
        filter->setName(myName->text());
        filter->setFilter(myScript->toPlainText());
        myList->item(myFilterIndex)->setText(filter->name());
    }
}

void CREFilterDialog::currentRowChanged(int currentRow)
{
    saveCurrentFilter();

    myFilterIndex = -1;
    if (currentRow >= 0 && currentRow < myFilters.filters().size())
    {
        const CREFilterDefinition* filter = myFilters.filters()[currentRow];
        myName->setText(filter->name());
        myScript->setText(filter->filter());
        myFilterIndex = currentRow;
    }
}
