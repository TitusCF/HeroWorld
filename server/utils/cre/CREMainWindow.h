#include <Qt>

#include <QMainWindow>

#include "CREResourcesWindow.h"

class QMdiArea;
class QAction;
class QMenu;
class QLabel;
class CREArtifactWindow;
class CREArchetypeWindow;
class CRETreasureWindow;
class CREAnimationWindow;
class CREFormulaeWindow;
class CREMapInformationManager;
class QuestManager;
class MessageManager;
class ResourcesManager;

class CREMainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        CREMainWindow();

    signals:
        void updateFilters();
        void updateReports();
        void commitData();

    private:
        QMdiArea* myArea;

        void createActions();
        void createMenus();

        QMenu* myOpenMenu;
        QMenu* mySaveMenu;

        QAction* myOpenArtifacts;
        QAction* myOpenArchetypes;
        QAction* myOpenTreasures;
        QAction* myOpenAnimations;
        QAction* myOpenFormulae;
        QAction* myOpenFaces;
        QAction* myOpenMaps;
        QAction* myOpenResources;
        QAction* myOpenExperience;
        QAction* myOpenQuests;
        QAction* myOpenMessages;
        QAction* mySaveFormulae;
        QAction* mySaveQuests;
        QAction* mySaveMessages;
        QAction* myReportDuplicate;
        QAction* myReportSpellDamage;
        QAction* myReportAlchemy;
        QAction* myReportSpells;
        QAction* myReportPlayer;
        QAction* myReportSummon;
        QAction* myReportShops;
        QAction *myReportQuests;
        QAction* myToolSmooth;
        QAction* myToolHPBar;
        QAction* myToolCombatSimulator;
        QLabel* myMapBrowseStatus;
        CREMapInformationManager* myMapManager;
        QuestManager* myQuestManager;
        MessageManager* myMessageManager;
        ResourcesManager* myResourcesManager;

    protected:
        void closeEvent(QCloseEvent* event);
        void doResourceWindow(DisplayMode mode);

    private slots:
        void onOpenArtifacts();
        void onOpenArchetypes();
        void onOpenTreasures();
        void onOpenAnimations();
        void onOpenFormulae();
        void onOpenFaces();
        void onOpenMaps();
        void onOpenQuests();
        void onOpenResources();
        void onOpenMessages();
        void onOpenExperience();
        void onSaveFormulae();
        void onSaveQuests();
        void onSaveMessages();
        void onReportDuplicate();
        void onReportSpellDamage();
        void onReportAlchemy();
        void onReportSpells();
        void onReportPlayer();
        void onReportSummon();
        void onReportShops();
        void onReportQuests();
        void onToolSmooth();
        void onToolCombatSimulator();
        void onToolBarMaker();
        void browsingMap(const QString& path);
        void browsingFinished();
        void onFiltersModified();
        void onReportsModified();
};
