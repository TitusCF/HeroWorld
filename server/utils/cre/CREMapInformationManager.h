#ifndef CLASS_CRE_MAP_INFORMATION_MANAGER_H
#define CLASS_CRE_MAP_INFORMATION_MANAGER_H

#include <QtCore>
#include "CREMapInformation.h"

extern "C" {
#include "global.h"
}

class MessageManager;
class QuestManager;

class CREMapInformationManager : public QObject
{
    Q_OBJECT

    public:
        CREMapInformationManager(QObject* parent, MessageManager* messageManager, QuestManager* questManager);
        virtual ~CREMapInformationManager();

        bool browseFinished() const;
        void start();
        void cancel();

        QList<CREMapInformation*> allMaps();
        QList<CREMapInformation*> getArchetypeUse(const archetype* arch);
        QList<CREMapInformation*> getMapsForRegion(const QString& region);

    signals:
        void browsingMap(const QString& path);
        void finished();

    protected:
        MessageManager* myMessageManager;
        QuestManager* myQuestManager;
        QHash<QString, CREMapInformation*> myInformation;
        QMultiHash<QString, CREMapInformation*> myArchetypeUse;
        QStringList myToProcess;
        int myCurrentMap;
        QFuture<void> myWorker;
        bool myCancelled;
        QMutex myLock;
        QHash<QString, qint64> myExperience;

        void browseMaps();
        void process(const QString& path);
        void checkInventory(const object* item, CREMapInformation* information);
        void loadCache();
        void storeCache();
        CREMapInformation* getOrCreateMapInformation(const QString& path);
        void addArchetypeUse(const QString& name, CREMapInformation* map);
        void checkEvent(const object* item, CREMapInformation* map);
};

#endif // CLASS_CRE_MAP_INFORMATION_MANAGER_H
