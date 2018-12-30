#include "CREStringListPanel.h"
#include <QtGui>

CREStringListPanel::CREStringListPanel(QWidget* parent) : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);
    myItems = new QListWidget(this);
    connect(myItems, SIGNAL(currentRowChanged(int)), this, SLOT(onCurrentItemChanged(int)));
    layout->addWidget(new QLabel(tr("Message:"), this), 0, 0, 1, 2);
    layout->addWidget(myItems, 1, 0, 1, 2);

    QPushButton* add = new QPushButton(tr("add"), this);
    connect(add, SIGNAL(clicked(bool)), this, SLOT(onAddItem(bool)));
    layout->addWidget(add, 2, 0);

    QPushButton* remove = new QPushButton(tr("remove"), this);
    connect(remove, SIGNAL(clicked(bool)), this, SLOT(onDeleteItem(bool)));
    layout->addWidget(remove, 2, 1);

    layout->addWidget(new QLabel(tr("Message:"), this), 3, 0);

    myTextEdit = new QTextEdit(this);
    connect(myTextEdit, SIGNAL(textChanged()), this, SLOT(onTextEditChanged()));
    layout->addWidget(myTextEdit, 3, 1);

    myCurrentLine = -1;
}

CREStringListPanel::~CREStringListPanel()
{
}

void CREStringListPanel::clearData()
{

}

void CREStringListPanel::setData(const QStringList& list)
{
    myCurrentLine = -1;
    myItems->clear();
    myItems->addItems(list);
    if (myTextEdit != NULL)
        myTextEdit->setText("");
}

QStringList CREStringListPanel::getData() const
{
    QStringList data;
    for (int i = 0; i < myItems->count(); i++)
        data.append(myItems->item(i)->text());
    return data;
}

void CREStringListPanel::onAddItem(bool)
{
    myItems->addItem("<item>");
    emit dataModified();
}

void CREStringListPanel::onDeleteItem(bool)
{
    if (myCurrentLine == -1 || myCurrentLine >= myItems->count())
        return;

    delete myItems->takeItem(myCurrentLine);
    myCurrentLine = -1;
    if (myTextEdit != NULL)
        myTextEdit->setText("");
    emit dataModified();
}

void CREStringListPanel::commitData()
{
    if (myCurrentLine == -1 || myCurrentLine >= myItems->count())
        return;

    myItems->item(myCurrentLine)->setText(myTextEdit->toPlainText());
    emit dataModified();
}

void CREStringListPanel::onCurrentItemChanged(int currentRow)
{
    commitData();
    if (currentRow == -1)
        return;
    myCurrentLine = currentRow;
    myTextEdit->setText(myItems->item(myCurrentLine)->text());
}

void CREStringListPanel::onTextEditChanged()
{
    if (myCurrentLine == -1)
        return;
    myItems->item(myCurrentLine)->setText(myTextEdit->toPlainText());
    emit dataModified();
}
