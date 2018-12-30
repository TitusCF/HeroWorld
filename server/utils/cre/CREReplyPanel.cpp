#include "CREReplyPanel.h"
#include <QtGui>

CREReplyPanel::CREReplyPanel(QWidget* parent) : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    myReplies = new QTreeWidget(this);
    QStringList headers;
    headers << "Text" << "Message" << "Type";
    myReplies->setHeaderLabels(headers);
    connect(myReplies, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(currentReplyChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    layout->addWidget(myReplies, 0, 0, 4, 4);

    QPushButton* add = new QPushButton(tr("add"), this);
    connect(add, SIGNAL(clicked(bool)), this, SLOT(onAddItem(bool)));
    layout->addWidget(add, 4, 0, 1, 2);

    QPushButton* remove = new QPushButton(tr("remove"), this);
    connect(remove, SIGNAL(clicked(bool)), this, SLOT(onDeleteItem(bool)));
    layout->addWidget(remove, 4, 2, 1, 2);

    myText = new QLineEdit(this);
    connect(myText, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
    layout->addWidget(myText, 5, 0);
    myMessage = new QLineEdit(this);
    connect(myMessage, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
    layout->addWidget(myMessage, 5, 1, 1, 2);
    myType = new QComboBox(this);
    myType->addItem(tr("say"));
    myType->addItem(tr("reply"));
    myType->addItem(tr("question"));
    connect(myType, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
    layout->addWidget(myType, 5, 3);
}

CREReplyPanel::~CREReplyPanel()
{
}

void CREReplyPanel::setData(const QList<QStringList>& data)
{
    myData = data;
    myReplies->clear();
    foreach(QStringList reply, data)
    {
        setText(new QTreeWidgetItem(myReplies), reply);
    }
}

QList<QStringList> CREReplyPanel::getData()
{
    return myData;
}

void CREReplyPanel::onAddItem(bool)
{
    QStringList add;
    add << "text" << "message" << "0";
    myData.append(add);
    setText(new QTreeWidgetItem(myReplies), add);
    myReplies->setCurrentItem(myReplies->topLevelItem(myData.size() - 1));
    emit dataModified();
}

void CREReplyPanel::onDeleteItem(bool)
{
    int idx = myReplies->indexOfTopLevelItem(myReplies->currentItem());
    if (idx < 0 || idx >= myData.size())
        return;

    myData.removeAt(idx);
    delete myReplies->takeTopLevelItem(idx);
    emit dataModified();
}

void CREReplyPanel::setText(QTreeWidgetItem* item, QStringList data)
{
    while (data.size() > 3)
        data.removeLast();
    while (data.size() < 3)
        data.append("");
    data[2] = myType->itemText(data[2].toInt());
    item->setText(0, data[0]);
    item->setText(1, data[1]);
    item->setText(2, data[2]);
}

void CREReplyPanel::onTextChanged(const QString &)
{
    updateItem();
}

void CREReplyPanel::onTypeChanged(int)
{
    updateItem();
}

void CREReplyPanel::updateItem()
{
    int idx = myReplies->indexOfTopLevelItem(myReplies->currentItem());
    if (idx < 0 || idx >= myData.size())
        return;

    QStringList data;
    data.append(myText->text());
    data.append(myMessage->text());
    data.append(QString::number(myType->currentIndex()));
    myData[idx] = data;
    setText(myReplies->currentItem(), data);
    emit dataModified();
}

void CREReplyPanel::currentReplyChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
    QTreeWidgetItem* item = myReplies->currentItem();
    if (!item)
        return;

    int idx = myReplies->indexOfTopLevelItem(item);
    if (idx < 0 || idx >= myData.size())
        return;

    QStringList data = myData[idx];
    myText->setText(data[0]);
    myMessage->setText(data[1]);
    if (data.size() > 2)
        myType->setCurrentIndex(data[2].toInt());
    else
        myType->setCurrentIndex(0);
}
