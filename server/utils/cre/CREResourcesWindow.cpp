#include <Qt>
#include <QtGui>

#include "CREResourcesWindow.h"
#include "CREUtils.h"
#include "CREPixmap.h"

#include "CREFilter.h"
#include "CREFilterDialog.h"
#include "CREFilterDefinition.h"

#include "CRESettings.h"

#include "CREReportDialog.h"
#include "CREReportDisplay.h"
#include "CREReportDefinition.h"

#include "CRETreeItemEmpty.h"
#include "CRETreeItemAnimation.h"
#include "CRETreeItemArchetype.h"
#include "CRETreeItemTreasure.h"
#include "CRETreeItemArtifact.h"
#include "CRETreeItemFormulae.h"
#include "CRETreeItemFace.h"
#include "CRETreeItemMap.h"
#include "CRETreeItemRegion.h"
#include "CRETreeItemQuest.h"
#include "CRETreeItemMessage.h"

#include "CREAnimationPanel.h"
#include "CREArchetypePanel.h"
#include "CRETreasurePanel.h"
#include "CREArtifactPanel.h"
#include "CREFormulaePanel.h"
#include "CREFacePanel.h"
#include "CREMapPanel.h"
#include "CRERegionPanel.h"
#include "CREQuestPanel.h"
#include "CREMessagePanel.h"

#include "CREWrapperObject.h"
#include "CREWrapperArtifact.h"
#include "CREWrapperFormulae.h"

#include "CREMapInformationManager.h"
#include "Quest.h"
#include "QuestManager.h"
#include "MessageFile.h"

#include "CREScriptEngine.h"

extern "C" {
#include "global.h"
#include "recipe.h"
#include "MessageManager.h"
#include "ResourcesManager.h"
}

CREResourcesWindow::CREResourcesWindow(CREMapInformationManager* store, QuestManager* quests, MessageManager* messages, ResourcesManager* resources, DisplayMode mode)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    myDisplay = mode;

    Q_ASSERT(store);
    myStore = store;
    Q_ASSERT(quests);
    myQuests = quests;
    Q_ASSERT(messages);
    myMessages = messages;
    Q_ASSERT(resources);
    myResources = resources;

    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* layout = new QVBoxLayout(this);

    myFiltersMenu = new QMenu(this);
    QHBoxLayout* buttons = new QHBoxLayout();
    myFilterButton = new QPushButton(tr("Filter..."), this);
    myFilterButton->setMenu(myFiltersMenu);
    buttons->addWidget(myFilterButton);

    myReportsMenu = new QMenu(this);
    QPushButton* report = new QPushButton(tr("Report"), this);
    report->setMenu(myReportsMenu);
    buttons->addWidget(report);

    layout->addLayout(buttons);

    mySplitter = new QSplitter(this);
    layout->addWidget(mySplitter);
    myTree = new QTreeWidget(this);
    mySplitter->addWidget(myTree);
    myTree->setIconSize(QSize(32, 32));
    myTree->setHeaderLabel(tr("All resources"));
//    myTree->sortByColumn(0, Qt::AscendingOrder);

//    myTree->setSortingEnabled(true);
    myTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(myTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(treeCustomMenu(const QPoint&)));
    connect(myTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(tree_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

    /* dummy panel to display for empty items */
    CREPanel* dummy = new CREPanel();
    QVBoxLayout* dl = new QVBoxLayout(dummy);
    dl->addWidget(new QLabel(tr("No details available."), dummy));
    addPanel("(dummy)", dummy);
    dummy->setVisible(true);
    myCurrentPanel = dummy;

    fillData();

    connect(&myFiltersMapper, SIGNAL(mapped(QObject*)), this, SLOT(onFilterChange(QObject*)));
    updateFilters();
    connect(&myReportsMapper, SIGNAL(mapped(QObject*)), this, SLOT(onReportChange(QObject*)));
    updateReports();

    QApplication::restoreOverrideCursor();
}

CREResourcesWindow::~CREResourcesWindow()
{
    qDeleteAll(myTreeItems);
    myTreeItems.clear();
    qDeleteAll(myDisplayedItems);
    myDisplayedItems.clear();
}

void CREResourcesWindow::fillData()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    myTree->clear();
    qDeleteAll(myTreeItems);
    myTreeItems.clear();
    qDeleteAll(myDisplayedItems);
    myDisplayedItems.clear();

    QString title;
    if (myDisplay & DisplayArchetypes)
    {
        title = tr("Archetypes");
        fillArchetypes();
    }
    if (myDisplay & DisplayAnimations)
    {
        title = tr("Animations");
        fillAnimations();
    }
    if (myDisplay & DisplayTreasures)
    {
        title = tr("Treasures");
        fillTreasures();
    }
    if (myDisplay & DisplayFormulae)
    {
        title = tr("Formulae");
        fillFormulae();
    }
    if (myDisplay & DisplayArtifacts)
    {
        title = tr("Artifacts");
        fillArtifacts();
    }
    if (myDisplay & DisplayFaces)
    {
        title = tr("Faces");
        fillFaces();
    }
    if (myDisplay & DisplayMaps)
    {
        title = tr("Maps");
        fillMaps();
    }
    if (myDisplay & DisplayQuests)
    {
        title = tr("Quests");
        fillQuests();
    }
    if (myDisplay & DisplayMessage)
    {
        title = tr("NPC dialogs");
        fillMessages();
    }

    if (myDisplay == DisplayAll)
        title = tr("All resources");

    if (myTree->topLevelItemCount() == 1)
        myTree->topLevelItem(0)->setExpanded(true);

    setWindowTitle(title);

    myTree->resizeColumnToContents(0);

    QApplication::restoreOverrideCursor();
}

void CREResourcesWindow::commitData()
{
    if (myCurrentPanel != NULL)
        myCurrentPanel->commitData();
}

void CREResourcesWindow::tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
    if (!current || current->data(0, Qt::UserRole).value<void*>() == NULL)
        return;
    CRETreeItem* item = reinterpret_cast<CRETreeItem*>(current->data(0, Qt::UserRole).value<void*>());
    if (!item)
        return;

    commitData();

    CREPanel* newPanel = myPanels[item->getPanelName()];
    if (!newPanel)
    {
//        printf("no panel for %s\n", qPrintable(item->getPanelName()));
        return;
    }

    item->fillPanel(newPanel);

    if (myCurrentPanel != newPanel)
    {
        if (myCurrentPanel)
            myCurrentPanel->setVisible(false);
        newPanel->setVisible(true);
        myCurrentPanel = newPanel;
    }
}

void CREResourcesWindow::fillAnimations()
{
    QTreeWidgetItem* animationsNode = CREUtils::animationNode(NULL);
    myTreeItems.append(new CRETreeItemEmpty());
    animationsNode->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    myTree->addTopLevelItem(animationsNode);

    QTreeWidgetItem* item;

    QStringList animations = myResources->allAnimations();
    // There is the "bug" animation to consider
    foreach(QString name, animations)
    {
        const animations_struct* anim = myResources->animation(name);
        item = CREUtils::animationNode(anim, animationsNode);
        myTreeItems.append(new CRETreeItemAnimation(anim));
        item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    }

    addPanel("Animation", new CREAnimationPanel());
}

void CREResourcesWindow::fillTreasures()
{
    QTreeWidgetItem* item, *sub;
    const treasurelist* list;
    const treasure* treasure;

    QTreeWidgetItem* treasures = CREUtils::treasureNode(NULL);
    myTreeItems.append(new CRETreeItemEmpty());
    treasures->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    myTree->addTopLevelItem(treasures);

    QStringList names = myResources->treasureLists();

    foreach(QString name, names)
    {
        list = myResources->treasureList(name);
        item = CREUtils::treasureNode(list, treasures);

        myTreeItems.append(new CRETreeItemTreasure(list));
        item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
        //item->setData(0, Qt::UserRole, allTreasures[t]);
        if (list->total_chance != 0)
            item->setText(1, QString::number(list->total_chance));

        for (treasure = list->items; treasure; treasure = treasure->next)
        {
            sub = CREUtils::treasureNode(treasure, list, item);
            if (treasure->chance)
                sub->setText(1, QString::number(treasure->chance));
            myTreeItems.append(new CRETreeItemTreasure(list));
            sub->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
        }
    }

    addPanel("Treasure", new CRETreasurePanel());
}

void CREResourcesWindow::fillArchetypes()
{
    QTreeWidgetItem* item, *root, *sub;
    const archt* arch;
    int added = 0, count = 0;

    root = CREUtils::archetypeNode(NULL);
    myTreeItems.append(new CRETreeItemEmpty());
    root->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    myTree->addTopLevelItem(root);

    CREWrapperObject* wrapper = NULL;

    QStringList archs = myResources->archetypes();

    foreach(QString name, archs)
    {
        arch = myResources->archetype(name);
        count++;
        if (!wrapper)
            wrapper = new CREWrapperObject();
        wrapper->setObject(&arch->clone);
        if (!myFilter.showItem(wrapper))
            continue;

        item = CREUtils::archetypeNode(arch, root);
        myTreeItems.append(new CRETreeItemArchetype(arch));
        item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));

        for (archt* more = arch->more; more; more = more->more)
        {
            sub = CREUtils::archetypeNode(more, item);
            myTreeItems.append(new CRETreeItemArchetype(more));
            sub->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
        }
        myDisplayedItems.append(wrapper);
        wrapper = NULL;
        added++;
    }

    delete wrapper;
    addPanel("Archetype", new CREArchetypePanel(myStore));
    if (added == count)
        root->setText(0, tr("%1 [%2 items]").arg(root->text(0)).arg(count));
    else
        root->setText(0, tr("%1 [%2 items out of %3]").arg(root->text(0)).arg(added).arg(count));
}

void CREResourcesWindow::fillFormulae()
{
    const recipe* recipe;
    QTreeWidgetItem* root, *form, *sub;
    CREWrapperFormulae* wrapper = NULL;
    int count = 0, added = 0, subCount, subAdded;

    form = new QTreeWidgetItem(myTree, QStringList(tr("Formulae")));
    myTreeItems.append(new CRETreeItemEmpty());
    form->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
//    myTree->addTopLevelItem(form);

    for (int ing = 1; ing <= myResources->recipeMaxIngredients() ; ing++)
    {
        root = new QTreeWidgetItem(form, QStringList(tr("%1 ingredients").arg(ing)));
        subCount = 0;
        subAdded = 0;

        QStringList recipes = myResources->recipes(ing);

        foreach(QString name, recipes)
        {
            recipe = myResources->recipe(ing, name);
            subCount++;
            count++;
            if (!wrapper)
                wrapper = new CREWrapperFormulae();
            wrapper->setFormulae(recipe);
            if (!myFilter.showItem(wrapper))
                continue;

            sub = CREUtils::formulaeNode(recipe, root);
            myTreeItems.append(new CRETreeItemFormulae(recipe));
            sub->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
            myDisplayedItems.append(wrapper);
            wrapper = NULL;
            subAdded++;
            added++;
        }
        if (subCount == subAdded)
            root->setText(0, tr("%1 [%2 items]").arg(root->text(0)).arg(subCount));
        else
            root->setText(0, tr("%1 [%2 items out of %3]").arg(root->text(0)).arg(added).arg(subCount));
    }

    delete wrapper;
    addPanel("Formulae", new CREFormulaePanel());
    if (added == count)
        form->setText(0, tr("%1 [%2 items]").arg(form->text(0)).arg(count));
    else
        form->setText(0, tr("%1 [%2 items out of %3]").arg(form->text(0)).arg(added).arg(count));
}

void CREResourcesWindow::fillArtifacts()
{
    QTreeWidgetItem* item, *root, *sub;
    artifactlist* list;
    const typedata* data;
    int count = 0, added = 0;

    root = new QTreeWidgetItem(myTree, QStringList(tr("Artifacts")));
    myTreeItems.append(new CRETreeItemEmpty());
    root->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));

    CREWrapperArtifact wrapper;

    for (list = first_artifactlist; list; list = list->next)
    {
        int subCount = 0, subAdded = 0;
        data = get_typedata(list->type);

        item = new QTreeWidgetItem(root, QStringList(data ? data->name : tr("type %1").arg(list->type)));

        for (artifact* art = list->items; art; art = art->next)
        {
            count++;
            subCount++;
            wrapper.setArtifact(art);
            if (!myFilter.showItem(&wrapper))
                continue;

            sub = CREUtils::artifactNode(art, item);
            myTreeItems.append(new CRETreeItemArtifact(art));
            sub->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
            added++;
            subAdded++;
        }

        if (subCount == subAdded)
            item->setText(0, tr("%1 [%2 items]").arg(item->text(0)).arg(subCount));
        else
            item->setText(0, tr("%1 [%2 items out of %3]").arg(item->text(0)).arg(subAdded).arg(subCount));
    }

    addPanel("Artifact", new CREArtifactPanel());
    if (added == count)
        root->setText(0, tr("%1 [%2 items]").arg(root->text(0)).arg(count));
    else
        root->setText(0, tr("%1 [%2 items out of %3]").arg(root->text(0)).arg(added).arg(count));
}

void CREResourcesWindow::fillFaces()
{
    QTreeWidgetItem* item, *root;
    const New_Face* face;

    root = CREUtils::faceNode(NULL);
    myTreeItems.append(new CRETreeItemEmpty());
    root->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    myTree->addTopLevelItem(root);

    QStringList faces = myResources->faces();

    foreach(QString name, faces)
    {
        face = myResources->face(name);
        item = CREUtils::faceNode(face, root);
        myTreeItems.append(new CRETreeItemFace(face));
        item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    }

    addPanel("Face", new CREFacePanel());
}

void CREResourcesWindow::fillMaps()
{
    bool full = false;
    if (myDisplay == DisplayMaps)
    {
        QStringList headers;
        headers << tr("Maps") << tr("Experience");
        myTree->setHeaderLabels(headers);
        myTree->sortByColumn(0, Qt::AscendingOrder);
        full = true;
    }

    QTreeWidgetItem* regionNode, *root, *leaf;

    root = CREUtils::mapNode(NULL);
    myTreeItems.append(new CRETreeItemEmpty());
    root->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    myTree->addTopLevelItem(root);

    region* reg;
    for (reg = first_region; reg; reg = reg->next)
    {
        QList<CREMapInformation*> maps = myStore->getMapsForRegion(reg->name);
        regionNode = CREUtils::regionNode(reg->name, maps.size(), root);
        myTreeItems.append(new CRETreeItemRegion(reg));
        regionNode->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
        foreach(CREMapInformation* map, maps)
        {
            leaf = CREUtils::mapNode(map, regionNode);
            myTreeItems.append(new CRETreeItemMap(map));
            leaf->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
            if (full)
                leaf->setText(1, tr("%1").arg(QString::number(map->experience()), 20));

            /** @todo clean at some point - the issue is wrapper's ownership */
            myDisplayedItems.append(map->clone());
        }
    }

    if (full)
    {
        root->setExpanded(true);
        myTree->resizeColumnToContents(0);
        myTree->resizeColumnToContents(1);
    }

    addPanel("Region", new CRERegionPanel());
    addPanel("Map", new CREMapPanel());
}

void CREResourcesWindow::fillQuests()
{
    QTreeWidgetItem* item, *root;

    root = CREUtils::questsNode();
    myTreeItems.append(new CRETreeItemQuest(NULL, root, this));
    root->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    myTree->addTopLevelItem(root);

    QStringList codes;
    const New_Face* face;

    foreach(Quest* quest, myQuests->quests())
    {
      face = myResources->face(quest->face());
      if (face != NULL)
        quest->setFaceNumber(face->number);
        codes.append(quest->code());
    }
    codes.sort();

    foreach(QString code, codes)
    {
        Quest* quest = myQuests->getByCode(code);
        item = CREUtils::questNode(quest, root);
        myTreeItems.append(new CRETreeItemQuest(quest, item, this));
        item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    }

    addPanel("Quest", new CREQuestPanel(myQuests, myMessages));
}

void CREResourcesWindow::fillMessages()
{
    QTreeWidgetItem* item, *root;

    root = CREUtils::messagesNode();
    myTreeItems.append(new CRETreeItemEmpty());
    root->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    myTree->addTopLevelItem(root);

    foreach(MessageFile* message, myMessages->messages())
    {
        item = CREUtils::messageNode(message, root);
        myTreeItems.append(new CRETreeItemMessage(message));
        item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(myTreeItems.last()));
    }

    addPanel("Message", new CREMessagePanel(myMessages, myQuests));
}

void CREResourcesWindow::addPanel(QString name, CREPanel* panel)
{
    panel->setVisible(false);
    myPanels[name] = panel;
    mySplitter->addWidget(panel);
}

void CREResourcesWindow::onFilter()
{
    CREFilterDialog dlg;
    if (dlg.exec() != QDialog::Accepted)
        return;

    /* sending this signal will ultimately call our own updateFilters() */
    emit filtersModified();
}

void CREResourcesWindow::onReport()
{
    CREReportDialog dlg;
    if (dlg.exec() != QDialog::Accepted)
        return;

    /* sending this signal will ultimately call our own updateReports() */
    emit reportsModified();
}

void CREResourcesWindow::updateFilters()
{
    CRESettings settings;
    settings.loadFilters(myFilters);

    myFiltersMenu->clear();

    if (myFilters.filters().size() > 0)
    {
        QAction* clear = new QAction(tr("(none)"), this);
        connect(clear, SIGNAL(triggered()), this, SLOT(clearFilter()));
        myFiltersMenu->addAction(clear);

        foreach(CREFilterDefinition* filter, myFilters.filters())
        {
            QAction* a = new QAction(filter->name(), this);
            myFiltersMenu->addAction(a);
            myFiltersMapper.setMapping(a, filter);
            connect(a, SIGNAL(triggered()), &myFiltersMapper, SLOT(map()));
        }

        myFiltersMenu->addSeparator();
    }

    QAction* quick = new QAction(tr("Quick filter..."), this);
    connect(quick, SIGNAL(triggered()), this, SLOT(onQuickFilter()));
    myFiltersMenu->addAction(quick);
    QAction* dialog = new QAction(tr("Filters definition..."), this);
    connect(dialog, SIGNAL(triggered()), this, SLOT(onFilter()));
    myFiltersMenu->addAction(dialog);

    clearFilter();
}

void CREResourcesWindow::onFilterChange(QObject* object)
{
    CREFilterDefinition* filter = qobject_cast<CREFilterDefinition*>(object);
    if (filter == NULL)
        return;
    myFilter.setFilter(filter->filter());
    fillData();
    myFilterButton->setText(tr("Filter: %1").arg(filter->name()));
}

void CREResourcesWindow::onQuickFilter()
{
    bool ok;
    QString filter = QInputDialog::getText(this, tr("Quick filter"), tr("Filter:"), QLineEdit::Normal, myFilter.filter(), &ok);
    if (!ok)
        return;
    if (filter.isEmpty())
    {
        clearFilter();
        return;
    }
    myFilter.setFilter(filter);
    fillData();
    myFilterButton->setText(tr("Filter: %1").arg(filter));
}

void CREResourcesWindow::clearFilter()
{
    myFilter.setFilter(QString());
    fillData();
    myFilterButton->setText(tr("Filter..."));
}

void CREResourcesWindow::updateReports()
{
    CRESettings settings;
    settings.loadReports(myReports);

    myReportsMenu->clear();

    if (myReports.reports().size() > 0)
    {
        foreach(CREReportDefinition* report, myReports.reports())
        {
            QAction* a = new QAction(report->name(), this);
            myReportsMenu->addAction(a);
            myReportsMapper.setMapping(a, report);
            connect(a, SIGNAL(triggered()), &myReportsMapper, SLOT(map()));
        }

        myReportsMenu->addSeparator();
    }

    QAction* dialog = new QAction(tr("Reports definition..."), this);
    connect(dialog, SIGNAL(triggered()), this, SLOT(onReport()));
    myReportsMenu->addAction(dialog);
}

void CREResourcesWindow::onReportChange(QObject* object)
{
    CREReportDefinition* report = qobject_cast<CREReportDefinition*>(object);
    if (report == NULL)
        return;

    QProgressDialog progress(tr("Generating report..."), tr("Abort report"), 0, myDisplayedItems.size() * 2, this);
    progress.setWindowTitle(tr("Report: '%1'").arg(report->name()));
    progress.setWindowModality(Qt::WindowModal);

    QStringList headers = report->header().split("\n");
    QStringList fields = report->itemDisplay().split("\n");
    QString sort = report->itemSort();

    QString text("<table><thead><tr>");

    foreach(QString header, headers)
    {
        text += "<th>" + header + "</th>";
    }
    text += "</tr></thead><tbody>";

    CREScriptEngine engine;

    progress.setLabelText(tr("Sorting items..."));

    engine.pushContext();

    sort = "(function(left, right) { return " + sort + "; })";
    QScriptValue sortFun = engine.evaluate(sort);
    QScriptValueList args;

    QList<QObject*> data;
    int pos;
    for (int i = 0; i < myDisplayedItems.size(); i++)
    {
        if (progress.wasCanceled())
            return;

        args.clear();

        QScriptValue left = engine.newQObject(myDisplayedItems[i]);
        args.append(left);
//        engine.globalObject().setProperty("left", left);

        pos = 0;
        while (pos < data.size())
        {
            QScriptValue right = engine.newQObject(data[pos]);
            args.push_back(right);
            //engine.globalObject().setProperty("right", right);
            bool still = sortFun.call(QScriptValue(), args).toBoolean();
            args.pop_back();
            if (still == false)
                break;
            pos++;
        }
        if (pos == data.size())
            data.append(myDisplayedItems[i]);
        else
            data.insert(pos, myDisplayedItems[i]);

        progress.setValue(i + 1);
    }
    engine.popContext();

    progress.setLabelText(tr("Generating items text..."));
    foreach(QObject* item, data)
    {
        if (progress.wasCanceled())
            return;

        text += "<tr>";

        engine.pushContext();
        QScriptValue engineValue = engine.newQObject(item);
        engine.globalObject().setProperty("item", engineValue);

        foreach(QString field, fields)
        {
            text += "<td>";
            QString data = engine.evaluate(field).toString();
            if (!engine.hasUncaughtException())
            {
                text += data;
            }
            text += "</td>\n";
        }
        engine.popContext();
        text += "</tr>\n";

        progress.setValue(progress.value() + 1);
    }
    text += "</tbody></table>";
    qDebug() << "report finished";

    CREReportDisplay display(text);
    display.exec();
}

void CREResourcesWindow::fillItem(const QPoint& pos, QMenu* menu)
{
    QTreeWidgetItem* node = myTree->itemAt(pos);
    if (!node || node->data(0, Qt::UserRole).value<void*>() == NULL)
        return;
    CRETreeItem* item = reinterpret_cast<CRETreeItem*>(node->data(0, Qt::UserRole).value<void*>());
    if (!item)
        return;

    item->fillContextMenu(menu);
}

void CREResourcesWindow::treeCustomMenu(const QPoint & pos)
{
    QMenu menu;

    if (myDisplay & DisplayMessage)
    {
        QAction* addMessage = new QAction("add message", &menu);
        connect(addMessage, SIGNAL(triggered(bool)), this, SLOT(addMessage(bool)));
        menu.addAction(addMessage);
    }

    if (myDisplay & DisplayQuests)
    {
        QAction* addQuest = new QAction("add quest", &menu);
        connect(addQuest, SIGNAL(triggered(bool)), this, SLOT(addQuest(bool)));
        menu.addAction(addQuest);
    }

    fillItem(pos, &menu);

    if (menu.actions().size() == 0)
        return;
    menu.exec(myTree->mapToGlobal(pos));
}

void CREResourcesWindow::deleteQuest(Quest* /*quest*/)
{
    /** @todo doesn't work for some reason, signal issue probably */
    /*
    myQuests->quests().removeAll(quest);
    fillData();
    delete quest;
     */
}

void CREResourcesWindow::addQuest(bool)
{
    Quest* quest = new Quest();
    quest->setCode("(new quest)");
    myQuests->quests().append(quest);
    fillData();
}

void CREResourcesWindow::addMessage(bool)
{
    MessageFile* file = new MessageFile("<new file>");
    file->setModified();
    myMessages->messages().append(file);
    fillData();
}

const ResourcesManager* CREResourcesWindow::resourcesManager() const
{
  return myResources;
}