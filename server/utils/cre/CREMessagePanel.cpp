#include <QtGui>
#include <qdir.h>
#include "CREMessagePanel.h"
#include "CREFilterDefinition.h"
#include "MessageFile.h"
#include "CRERulePanel.h"
#include "CREMapInformation.h"
#include "MessageManager.h"

CREMessagePanel::CREMessagePanel(const MessageManager* manager, const QuestManager* quests)
{
    Q_ASSERT(manager != NULL);
    myMessageManager = manager;

    QVBoxLayout* main = new QVBoxLayout(this);
    QTabWidget* tab = new QTabWidget(this);
    main->addWidget(tab);

    QWidget* details = new QWidget(this);
    tab->addTab(details, tr("Details"));


    QGridLayout* layout = new QGridLayout(details);

    int line = 0;

    layout->addWidget(new QLabel(tr("Path:"), this), line, 0);
    myPath = new QLineEdit(this);
    layout->addWidget(myPath, line++, 1);

    layout->addWidget(new QLabel(tr("Location:"), this), line, 0, 1, 2);
    myLocation = new QLineEdit(this);
    layout->addWidget(myLocation, line++, 1, 1, 2);

    QGroupBox* box = new QGroupBox(tr("Rules"));
    layout->addWidget(box, line++, 0, 1, 4);

    QGridLayout* rules = new QGridLayout();
    box->setLayout(rules);

    myRules = new QTreeWidget();
    rules->addWidget(myRules, 0, 0, 4, 4);
    QStringList labels;
    labels << tr("match") << tr("pre") << tr("message") << tr("post") << tr("replies") << tr("include");
    myRules->setHeaderLabels(labels);
    myRules->setWordWrap(true);
    connect(myRules, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

    QHBoxLayout* buttons = new QHBoxLayout();

    QPushButton* add = new QPushButton(tr("add rule"), this);
    buttons->addWidget(add);
    connect(add, SIGNAL(clicked(bool)), this, SLOT(onAddRule(bool)));
    QPushButton* remove = new QPushButton(tr("remove rule"), this);
    buttons->addWidget(remove);
    connect(remove, SIGNAL(clicked(bool)), this, SLOT(onDeleteRule(bool)));

    QPushButton* up = new QPushButton(tr("move up"), this);
    buttons->addWidget(up);
    connect(up, SIGNAL(clicked(bool)), this, SLOT(onMoveUp(bool)));
    QPushButton* down = new QPushButton(tr("move down"), this);
    buttons->addWidget(down);
    connect(down, SIGNAL(clicked(bool)), this, SLOT(onMoveDown(bool)));

    QPushButton* copy = new QPushButton(tr("copy"), this);
    buttons->addWidget(copy);
    connect(copy, SIGNAL(clicked(bool)), this, SLOT(onDuplicate(bool)));

    rules->addLayout(buttons, 4, 0, 1, 4);

    myRulePanel = new CRERulePanel(manager, quests, this);
    connect(myRulePanel, SIGNAL(currentRuleModified()), this, SLOT(currentRuleModified()));
    rules->addWidget(myRulePanel, 5, 0, 4, 4);

    myUse = new QTreeWidget(this);
    tab->addTab(myUse, tr("Use"));
    myUse->setHeaderLabel(tr("Referenced by..."));

    myMessage = NULL;
}

CREMessagePanel::~CREMessagePanel()
{
}

QString toDisplay(const QList<QStringList>& list)
{
    QStringList data;
    foreach(QStringList item, list)
        data.append(item.join(" "));
    return data.join("\n");
}

void CREMessagePanel::setMessage(MessageFile* message)
{
    myPath->setText(message->path());
    /* can only change path when new file is created */
    myPath->setEnabled(message->path() == "<new file>");
    myLocation->setText(message->location());

    /* so the change handler won't do anything */
    myMessage = NULL;
    myRules->clear();
    myMessage = message;

    foreach(MessageRule* rule, myMessage->rules())
    {
        QStringList data;
        data << rule->match().join("\n");
        data << toDisplay(rule->preconditions());
        data << rule->messages().join("\n");
        data << toDisplay(rule->postconditions());
        data << toDisplay(rule->replies());
        data << rule->include();
        new QTreeWidgetItem(myRules, data);
    }

    if (myRules->topLevelItemCount() != 0)
        myDefaultBackground = myRules->topLevelItem(0)->background(0);

    myRulePanel->setMessageRule(NULL);

    myUse->clear();

    QTreeWidgetItem* root = NULL;
    if (myMessage->maps().length() > 0)
    {
        root = new QTreeWidgetItem(myUse, QStringList(tr("Maps")));
        root->setExpanded(true);
        foreach(CREMapInformation* map, myMessage->maps())
        {
            new QTreeWidgetItem(root, QStringList(map->path()));
        }
        root = NULL;
    }

    foreach(MessageFile* file, myMessageManager->messages())
    {
        bool got = false;
        foreach(MessageRule* rule, file->rules())
        {
            QStringList includes = rule->include();
            foreach(QString include, includes)
            {
                if (include.isEmpty())
                    continue;

                if (!include.startsWith('/'))
                {
                    int last = file->path().lastIndexOf(QDir::separator());
                    if (last == -1)
                        continue;
                    include = file->path().left(last + 1) + include;
                }

                if (include == message->path())
                {
                    if (root == NULL)
                    {
                        root = new QTreeWidgetItem(myUse, QStringList(tr("Messages")));
                        root->setExpanded(true);
                    }

                    new QTreeWidgetItem(root, QStringList(file->path()));

                    got = true;
                    break;
                }

                if (got)
                    break;
            }
        }

        if (got)
            break;
    }
}

void setBackgroundColor(QTreeWidgetItem* item, QBrush color)
{
    for (int c = 0; c < item->columnCount(); c++)
        item->setBackground(c, color);
}

void CREMessagePanel::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (!myMessage || myRules->topLevelItemCount() == 0)
        return;

    for (int r = 0; r < myMessage->rules().size(); r++)
    {
        setBackgroundColor(myRules->topLevelItem(r), myDefaultBackground);
    }

    int index = myRules->indexOfTopLevelItem(current);
    if (index < 0 || index >= myMessage->rules().size())
        return;

    MessageRule* rule = myMessage->rules()[index];

    myRulePanel->setMessageRule(rule);

    foreach(QStringList pre, rule->preconditions())
    {
        if (pre.size() < 3)
            continue;

        if (pre[0] != "token" && pre[0] != "npctoken")
            continue;

        QStringList acceptable = pre;
        acceptable.removeFirst();
        acceptable.removeFirst();

        for (int c = 0; c < myMessage->rules().size(); c++)
        {
            MessageRule* check = myMessage->rules()[c];

            if (check == rule)
                continue;

            bool match = false;
            foreach(QStringList post, check->postconditions())
            {
                if (post.size() < 3)
                    continue;
                if ((post[0] != "settoken" && post[0] != "setnpctoken") || post[1] != pre[1] || !acceptable.contains(post[2]))
                    continue;
                match = true;
                break;
            }

            if (match)
                setBackgroundColor(myRules->topLevelItem(c), Qt::red);
        }
    }

    foreach(QStringList post, rule->postconditions())
    {
        if (post.size() < 3)
            continue;

        if (post[0] != "settoken" && post[0] != "setnpctoken")
            continue;

        for (int c = 0; c < myMessage->rules().size(); c++)
        {
            MessageRule* check = myMessage->rules()[c];

            if (check == rule)
                continue;

            bool match = false;
            foreach(QStringList pre, check->preconditions())
            {
                if (pre.size() < 3)
                    continue;
                if ((pre[0] != "token" && pre[0] != "npctoken") || pre[1] != post[1])
                    continue;

                QStringList acceptable = pre;
                acceptable.removeFirst();
                acceptable.removeFirst();
                if (!acceptable.contains(post[2]))
                    continue;

                match = true;
                break;
            }

            if (match)
                setBackgroundColor(myRules->topLevelItem(c), Qt::blue);
        }
    }
}

void CREMessagePanel::currentRuleModified()
{
    int index = myRules->currentIndex().row();
    fillRuleItem(myRules->currentItem(), myMessage->rules()[index]);
    myMessage->rules()[index]->setModified();
}

void CREMessagePanel::fillRuleItem(QTreeWidgetItem* item, MessageRule* rule)
{
    item->setText(0, rule->match().join("\n"));
    item->setText(1, toDisplay(rule->preconditions()));
    item->setText(2, rule->messages().join("\n"));
    item->setText(3, toDisplay(rule->postconditions()));
    item->setText(4, toDisplay(rule->replies()));
    item->setText(5, rule->include().join("\n"));
}

void CREMessagePanel::onAddRule(bool)
{
    MessageRule* rule = new MessageRule();
    rule->match().append("*");
    rule->setModified(true);
    myMessage->rules().append(rule);
    new QTreeWidgetItem(myRules, QStringList("*"));
}

void CREMessagePanel::onDeleteRule(bool)
{
    int index = myRules->currentIndex().row();
    if (index < 0 || index > myMessage->rules().size())
        return;

    myMessage->rules().removeAt(index);
    delete myRules->takeTopLevelItem(index);
}

void CREMessagePanel::commitData()
{
    myMessage->setPath(myPath->text());
    myMessage->setLocation(myLocation->text());
}

void CREMessagePanel::onMoveUp(bool)
{
    int index = myRules->currentIndex().row();
    if (index <= 0 || index >= myMessage->rules().size())
        return;

    MessageRule* swap = myMessage->rules()[index - 1];
    myMessage->rules()[index - 1] = myMessage->rules()[index];
    myMessage->rules()[index] = swap;

    QTreeWidgetItem* item = myRules->takeTopLevelItem(index - 1);
    myRules->insertTopLevelItem(index, item);

    myMessage->setModified();
}

void CREMessagePanel::onMoveDown(bool)
{
    int index = myRules->currentIndex().row();
    if (index < 0 || index >= myMessage->rules().size() - 1)
        return;

    MessageRule* swap = myMessage->rules()[index + 1];
    myMessage->rules()[index + 1] = myMessage->rules()[index];
    myMessage->rules()[index] = swap;

    QTreeWidgetItem* item = myRules->takeTopLevelItem(index + 1);
    myRules->insertTopLevelItem(index, item);

    myMessage->setModified();
}

void CREMessagePanel::onDuplicate(bool)
{
    int index = myRules->currentIndex().row();
    if (index < 0 || index > myMessage->rules().size())
        return;

    const MessageRule* original = myMessage->rules()[index];

    MessageRule* rule = new MessageRule(*original);
    myMessage->rules().append(rule);
    fillRuleItem(new QTreeWidgetItem(myRules, rule->match()), rule);
}
