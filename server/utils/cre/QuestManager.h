#ifndef _QUESTMANAGER_H
#define	_QUESTMANAGER_H

#include <QList>
#include <QHash>
class Quest;

class QuestManager
{
    public:
        QuestManager();
        virtual ~QuestManager();

        void loadQuests();
        void saveQuests();
        QList<const Quest*> quests() const;
        QList<Quest*>& quests();
        Quest* getByCode(const QString& code);
        QStringList getFiles() const;
        QString getQuestFile(Quest* quest) const;
        void setQuestFile(Quest* quest, const QString& file);
        Quest* findByCode(const QString& code);

    private:
        QList<Quest*> myQuests;
        QHash<QString, QList<Quest*>*> myFiles;
        QHash<QString, QStringList> myIncludes;

        void loadQuestFile(const QString& filename);
        void saveQuestFile(const QString& filename);
        void addQuest(const QString& filename, Quest* quest);
};

#endif	/* _QUESTMANAGER_H */
