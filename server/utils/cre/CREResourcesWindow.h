#ifndef CRERESOURCESWINDOW_H
#define CRERESOURCESWINDOW_H

#include <QObject>
#include <QtGui>

#include "CREFilter.h"
#include "CREFilterDefinitionManager.h"
#include "CREReportDefinitionManager.h"
#include "CREPanel.h"
#include "CRETreeItem.h"

class CREMapInformationManager;
class Quest;
class QuestManager;
class MessageManager;
class ResourcesManager;

enum DisplayMode {
    DisplayAll = 0xFFFF,
    DisplayArchetypes = 1,
    DisplayAnimations = 2,
    DisplayTreasures = 4,
    DisplayFormulae = 8,
    DisplayArtifacts = 16,
    DisplayFaces = 32,
    DisplayMaps = 64,
    DisplayQuests = 128,
    DisplayMessage = 256
};

class CREResourcesWindow : public QWidget
{
    Q_OBJECT

    public:
        CREResourcesWindow(CREMapInformationManager* store, QuestManager* quests, MessageManager* messages, ResourcesManager* resources, DisplayMode mode = DisplayAll);
        virtual ~CREResourcesWindow();

        void deleteQuest(Quest* quest);

        const ResourcesManager* resourcesManager() const;

    public slots:
        void updateFilters();
        void updateReports();
        void commitData();

    signals:
        void filtersModified();
        void reportsModified();

    protected:
        QTreeWidget* myTree;
        CREPanel* myCurrentPanel;
        QHash<QString, QPointer<CREPanel> > myPanels;
        QSplitter* mySplitter;
        CREMapInformationManager* myStore;
        QuestManager* myQuests;
        MessageManager* myMessages;
        ResourcesManager* myResources;
        DisplayMode myDisplay;
        CREFilter myFilter;
        QList<QObject*> myDisplayedItems;
        QPushButton* myFilterButton;
        QMenu* myFiltersMenu;
        QSignalMapper myFiltersMapper;
        CREFilterDefinitionManager myFilters;
        QMenu* myReportsMenu;
        QSignalMapper myReportsMapper;
        CREReportDefinitionManager myReports;
        QList<CRETreeItem*> myTreeItems;

        void fillData();
        void fillAnimations();
        void fillTreasures();
        void fillArchetypes();
        void fillFormulae();
        void fillArtifacts();
        void fillFaces();
        void fillMaps();
        void fillQuests();
        void fillMessages();
        void addPanel(QString name, CREPanel* panel);
        void fillItem(const QPoint& pos, QMenu* menu);

    protected slots:
        void tree_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
        void onFilter();
        void onFilterChange(QObject* object);
        void onQuickFilter();
        void clearFilter();
        void onReport();
        void onReportChange(QObject* object);
        void treeCustomMenu(const QPoint & pos);
        void addQuest(bool);
        void addMessage(bool);
};

#endif // CRERESOURCESWINDOW_H
