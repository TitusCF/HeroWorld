#include "CREPrePostPanel.h"
#include "CRERulePanel.h"
#include <QtGui>
#include "QuestConditionScript.h"
#include "QuestManager.h"
#include "Quest.h"

CRESubItemList::CRESubItemList(QWidget* parent) : CRESubItemWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    mySubItems = new QListWidget(this);
    connect(mySubItems, SIGNAL(currentRowChanged(int)), this, SLOT(currentSubItemChanged(int)));
    layout->addWidget(mySubItems, 0, 0, 1, 2);

    QPushButton* addSubItem = new QPushButton(tr("add"), this);
    connect(addSubItem, SIGNAL(clicked(bool)), this, SLOT(onAddSubItem(bool)));
    layout->addWidget(addSubItem, 1, 0);

    QPushButton* delSubItem = new QPushButton(tr("delete"), this);
    connect(delSubItem, SIGNAL(clicked(bool)), this, SLOT(onDeleteSubItem(bool)));
    layout->addWidget(delSubItem, 1, 1);

    myItemEdit = new QLineEdit(this);
    connect(myItemEdit, SIGNAL(textChanged(const QString&)), this, SLOT(subItemChanged(const QString&)));
    layout->addWidget(myItemEdit, 2, 0, 1, 2);
}

void CRESubItemList::setData(const QStringList& data)
{
    myData = data;
    myData.takeFirst();
    mySubItems->clear();
    mySubItems->addItems(myData);
    myItemEdit->clear();
}

void CRESubItemList::currentSubItemChanged(int)
{
    if (mySubItems->currentItem())
        myItemEdit->setText(mySubItems->currentItem()->text());
}


void CRESubItemList::onAddSubItem(bool)
{
    myData.append("(item)");
    mySubItems->addItem("(item)");
    mySubItems->setCurrentRow(myData.size() - 1);

    emit dataModified(myData);
}

void CRESubItemList::onDeleteSubItem(bool)
{
    if (mySubItems->currentRow() < 0)
        return;

    myData.removeAt(mySubItems->currentRow());
    delete mySubItems->takeItem(mySubItems->currentRow());
    mySubItems->setCurrentRow(0);
    emit dataModified(myData);
}

void CRESubItemList::subItemChanged(const QString& text)
{
    if (mySubItems->currentRow() < 0)
        return;

    myData[mySubItems->currentRow()] = text;
    mySubItems->currentItem()->setText(text);
    emit dataModified(myData);
}

CRESubItemConnection::CRESubItemConnection(QWidget* parent) : CRESubItemWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("Connection number:"), this));
    myEdit = new QLineEdit(this);
    myEdit->setValidator(new QIntValidator(1, 65000, myEdit));
    connect(myEdit, SIGNAL(textChanged(const QString&)), this, SLOT(editChanged(const QString&)));
    layout->addWidget(myEdit);
    myWarning = new QLabel(this);
    myWarning->setVisible(false);
    layout->addWidget(myWarning);
    layout->addStretch();
}

void CRESubItemConnection::setData(const QStringList& data)
{
    if (data.size() < 2)
    {
        showWarning(tr("Not enough arguments"));
        return;
    }

    bool ok = false;
    int value = data[1].toInt(&ok);
    if (!ok || value <= 0 || value > 65000)
    {
        showWarning(tr("Invalid number %1, must be a number between 1 and 65000").arg(data[1]));
        value = 1;
    }

    myWarning->setVisible(false);
    myEdit->setText(QString::number(value));
}

void CRESubItemConnection::showWarning(const QString& warning)
{
    myWarning->setText(warning);
    myWarning->setVisible(true);
}

void CRESubItemConnection::editChanged(const QString& text)
{
    bool ok = false;
    int value = text.toInt(&ok);
    if (!ok || value <= 0 || value > 65000)
    {
        showWarning(tr("Invalid number %1, must be a number between 1 and 65000").arg(text));
        return;
    }

    myWarning->setVisible(false);
    emit dataModified(QStringList(text));
}

CRESubItemQuest::CRESubItemQuest(bool isPre, const QuestManager* quests, QWidget* parent) : CRESubItemWidget(parent)
{
    myQuests = quests;
    myIsPre = isPre;
    myInit = true;

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("Quest:"), this));

    myQuestList = new QComboBox(this);
    layout->addWidget(myQuestList);

    if (isPre)
    {
        myAtStep = new QRadioButton(tr("at step"), this);
        layout->addWidget(myAtStep);
        myFromStep = new QRadioButton(tr("from step"), this);
        layout->addWidget(myFromStep);
        myStepRange = new QRadioButton(tr("from step to step"), this);
        layout->addWidget(myStepRange);

        connect(myAtStep, SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
        connect(myFromStep, SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
        connect(myStepRange, SIGNAL(toggled(bool)), this, SLOT(checkToggled(bool)));
    }
    else
    {
        layout->addWidget(new QLabel(tr("new step:"), this));
        myAtStep = NULL;
        myFromStep = NULL;
    }

    myFirstStep = new QComboBox(this);
    layout->addWidget(myFirstStep);

    if (isPre)
    {
        mySecondStep = new QComboBox(this);
        layout->addWidget(mySecondStep);
    }
    else
        mySecondStep = NULL;

    layout->addStretch();

    connect(myQuestList, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedQuestChanged(int)));
    foreach(const Quest* quest, quests->quests())
    {
        myQuestList->addItem(quest->title() + " [" + quest->code() + "]", quest->code());
    }
    connect(myFirstStep, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedStepChanged(int)));
    if (mySecondStep)
        connect(mySecondStep, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedStepChanged(int)));

    myInit = false;
}

void CRESubItemQuest::setData(const QStringList& data)
{
    if (data.size() < 3)
        return;

    int index = myQuestList->findData(data[1], Qt::UserRole);
    if (index == -1)
    {
        return;
    }

    myInit = true;
    myQuestList->setCurrentIndex(index);

    if (!myIsPre)
    {
        myFirstStep->setCurrentIndex(myFirstStep->findData(data[2], Qt::UserRole));
        myInit = false;
        return;
    }

    QString steps = data[2];
    int idx = steps.indexOf('-');

    if (idx == -1)
    {
        int start = 0;
        if (steps.startsWith('='))
        {
            myAtStep->setChecked(true);
            start = 1;
        }
        else
            myFromStep->setChecked(true);

        myFirstStep->setCurrentIndex(myFirstStep->findData(steps.mid(start), Qt::UserRole));
    }
    else
    {
        myStepRange->setChecked(true);
        myFirstStep->setCurrentIndex(myFirstStep->findData(steps.left(idx), Qt::UserRole));
        mySecondStep->setCurrentIndex(mySecondStep->findData(steps.mid(idx + 1), Qt::UserRole));
    }

    myInit = false;
}

void CRESubItemQuest::selectedQuestChanged(int index)
{
    myFirstStep->clear();
    if (myIsPre)
        myFirstStep->addItem("(not started)", "0");

    if (mySecondStep)
        mySecondStep->clear();

    if (index < 0 || index >= myQuests->quests().size())
        return;

    const Quest* quest = myQuests->quests()[index];

    QString desc;
    foreach (const QuestStep* step, quest->steps())
    {
        desc = tr("%1 (%2)").arg(QString::number(step->step()), step->description().left(30));
        if (step->isCompletion())
            desc += " (end)";
        myFirstStep->addItem(desc, QString::number(step->step()));
        if (mySecondStep)
            mySecondStep->addItem(desc, QString::number(step->step()));
    }
}

void CRESubItemQuest::updateData()
{
    if (myInit)
        return;

    QStringList data;

    data << myQuestList->itemData(myQuestList->currentIndex(), Qt::UserRole).toString();

    if (myIsPre)
    {
        QString value;
        if (myStepRange->isChecked())
        {
            value = myFirstStep->itemData(myFirstStep->currentIndex(), Qt::UserRole).toString();
            value += "-";
            value += mySecondStep->itemData(mySecondStep->currentIndex(), Qt::UserRole).toString();
        }
        else
        {
            if (myAtStep->isChecked())
                value = "=";
            value += myFirstStep->itemData(myFirstStep->currentIndex(), Qt::UserRole).toString();
        }

        data << value;
    }
    else
    {
        data << myFirstStep->itemData(myFirstStep->currentIndex(), Qt::UserRole).toString();
    }

    emit dataModified(data);
}

void CRESubItemQuest::checkToggled(bool checked)
{
    if (checked == false)
        return;

    mySecondStep->setEnabled(myStepRange->isChecked());
    updateData();
}

void CRESubItemQuest::selectedStepChanged(int index)
{
    if (index == -1)
        return;

    updateData();
}

CRESubItemToken::CRESubItemToken(bool isPre, QWidget* parent) : CRESubItemWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("Token:"), this));
    myToken = new QLineEdit(this);
    layout->addWidget(myToken);
    connect(myToken, SIGNAL(textChanged(const QString&)), this, SLOT(tokenChanged(const QString&)));

    if (isPre)
    {
        layout->addWidget(new QLabel(tr("Values the token can be (one per line):"), this));
        myValues = new QTextEdit(this);
        myValues->setAcceptRichText(false);
        layout->addWidget(myValues);
        connect(myValues, SIGNAL(textChanged()), this, SLOT(valuesChanged()));
        myValue = NULL;
    }
    else
    {
        layout->addWidget(new QLabel(tr("Value to set for the token:"), this));
        myValue = new QLineEdit(this);
        layout->addWidget(myValue);
        connect(myValue, SIGNAL(textChanged(const QString&)), this, SLOT(tokenChanged(const QString&)));
        myValues = NULL;
    }
    layout->addStretch();
}

void CRESubItemToken::setData(const QStringList& data)
{
    QStringList copy(data);

    if (data.size() < 2)
    {
        myToken->clear();
        if (myValues != NULL)
            myValues->clear();
        if (myValue != NULL)
            myValue->clear();

        return;
    }
    copy.removeFirst();
    myToken->setText(copy.takeFirst());
    if (myValues != NULL)
        myValues->setText(copy.join("\n"));
    else if (copy.size() > 0)
        myValue->setText(copy[0]);
    else
        myValue->clear();
}

void CRESubItemToken::updateData()
{
    QStringList values;
    values.append(myToken->text());
    if (myValues != NULL)
        values.append(myValues->toPlainText().split("\n"));
    else
        values.append(myValue->text());
    emit dataModified(values);
}

void CRESubItemToken::tokenChanged(const QString&)
{
    updateData();
}

void CRESubItemToken::valuesChanged()
{
    updateData();
}


CREPrePostPanel::CREPrePostPanel(bool isPre, const QList<QuestConditionScript*> scripts, const QuestManager* quests, QWidget* parent) : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    myItems = new QListWidget(this);
    connect(myItems, SIGNAL(currentRowChanged(int)), this, SLOT(currentItemChanged(int)));
    layout->addWidget(myItems, 0, 0, 3, 2);

    QPushButton* addItem = new QPushButton(tr("add"), this);
    connect(addItem, SIGNAL(clicked(bool)), this, SLOT(onAddItem(bool)));
    layout->addWidget(addItem, 3, 0);

    QPushButton* delItem = new QPushButton(tr("delete"), this);
    connect(delItem, SIGNAL(clicked(bool)), this, SLOT(onDeleteItem(bool)));
    layout->addWidget(delItem, 3, 1);

    layout->addWidget(new QLabel(tr("Script:"), this), 0, 2);
    myChoices = new QComboBox(this);
    connect(myChoices, SIGNAL(currentIndexChanged(int)), this, SLOT(currentChoiceChanged(int)));

    mySubItemsStack = new QStackedWidget(this);

    for(int script = 0; script < scripts.size(); script++)
    {
        myChoices->addItem(scripts[script]->name());
        myChoices->setItemData(script, scripts[script]->comment(), Qt::ToolTipRole);
        mySubWidgets.append(createSubItemWidget(isPre, scripts[script], quests));
        mySubItemsStack->addWidget(mySubWidgets.last());
        connect(mySubWidgets.last(), SIGNAL(dataModified(const QStringList&)), this, SLOT(subItemChanged(const QStringList&)));
    }

    layout->addWidget(myChoices, 0, 3);

    layout->addWidget(mySubItemsStack, 1, 2, 3, 2);
}

CREPrePostPanel::~CREPrePostPanel()
{
}

QList<QStringList> CREPrePostPanel::getData()
{
    return myData;
}

void CREPrePostPanel::setData(const QList<QStringList> data)
{
    myItems->clear();

    myData = data;

    foreach(QStringList list, data)
    {
        if (list.size() > 0)
            myItems->addItem(list[0]);
        else
            myItems->addItem(tr("(empty)"));
    }
}

void CREPrePostPanel::onAddItem(bool)
{
    myData.append(QStringList("quest"));
    myItems->addItem("quest");
    myItems->setCurrentRow(myData.size() - 1);
    emit dataModified();
}

void CREPrePostPanel::onDeleteItem(bool)
{
    if (myItems->currentRow() < 0 || myItems->currentRow() >= myData.size())
        return;

    myData.removeAt(myItems->currentRow());
    delete myItems->takeItem(myItems->currentRow());
    myItems->setCurrentRow(0);
    emit dataModified();
}

void CREPrePostPanel::currentItemChanged(int index)
{
    if (index < 0 || index >= myData.size())
        return;

    QStringList data = myData[index];
    if (data.size() == 0)
        return;

    myChoices->setCurrentIndex(myChoices->findText(data[0]));

    mySubWidgets[myChoices->currentIndex()]->setData(data);
}

void CREPrePostPanel::currentChoiceChanged(int)
{
    if (myItems->currentRow() < 0 || myItems->currentRow() >= myData.size())
        return;

    QStringList& data = myData[myItems->currentRow()];
    if (data.size() == 0)
        data.append(myChoices->currentText());
    else
        data[0] = myChoices->currentText();
    myItems->currentItem()->setText(data[0]);

    mySubItemsStack->setCurrentIndex(myChoices->currentIndex());
    mySubWidgets[myChoices->currentIndex()]->setData(data);

    emit dataModified();
}

void CREPrePostPanel::subItemChanged(const QStringList& data)
{
    if (myItems->currentRow() < 0 || myItems->currentRow() >= myData.size())
        return;

    QStringList& item = myData[myItems->currentRow()];
    while (item.size() > 1)
        item.removeLast();
    item.append(data);

    emit dataModified();
}

CRESubItemWidget* CREPrePostPanel::createSubItemWidget(bool isPre, const QuestConditionScript* script, const QuestManager* quests)
{
    if (!isPre && script->name() == "connection")
        return new CRESubItemConnection(this);

    if (script->name() == "quest")
        return new CRESubItemQuest(isPre, quests, this);

    if (script->name() == "token" || script->name() == "settoken" || script->name() == "npctoken" || script->name() == "setnpctoken")
        return new CRESubItemToken(isPre, this);

    return new CRESubItemList(this);
}
