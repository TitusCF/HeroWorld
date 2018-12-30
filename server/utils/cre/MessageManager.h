#ifndef _MESSAGEMANAGER_H
#define	_MESSAGEMANAGER_H

#include <QList>
class MessageFile;

class QuestConditionScript;

class MessageManager
{
    public:
        MessageManager();
        virtual ~MessageManager();

        void loadMessages();
        void saveMessages();

        QList<MessageFile*>& messages();
        const QList<MessageFile*>& messages() const;
        MessageFile* findMessage(const QString& path);

        QList<QuestConditionScript*> preConditions() const;
        QList<QuestConditionScript*> postConditions() const;

    private:
        QList<MessageFile*> myMessages;
        QList<QuestConditionScript*> myPreConditions;
        QList<QuestConditionScript*> myPostConditions;

        QString loadScriptComment(const QString& path) const;
        void loadDirectory(const QString& directory);
        void findPrePost(const QString directory, QList<QuestConditionScript*>& list);
};

#endif	/* _MESSAGEMANAGER_H */

