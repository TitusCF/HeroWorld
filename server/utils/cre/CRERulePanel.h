#ifndef _CRERULEPANEL_H
#define	_CRERULEPANEL_H

#include <QObject>
#include <QtGui>
#include <QTabWidget>

class QListWidget;
class MessageRule;
class QPushButton;
class QLineEdit;
class CREStringListPanel;
class CREPrePostPanel;
class CREReplyPanel;
class MessageManager;
class QuestManager;

class CRERulePanel : public QTabWidget
{
    Q_OBJECT

    public:
        CRERulePanel(const MessageManager* manager, const QuestManager* quests, QWidget* parent);
        virtual ~CRERulePanel();

        void setMessageRule(MessageRule* rule);

    signals:
        void currentRuleModified();

    protected:
        MessageRule* myRule;
        QTextEdit* myMatches;
        CREPrePostPanel* myPre;
        CREStringListPanel* myMessages;
        CREPrePostPanel* myPost;
        CREReplyPanel* myReplies;
        QTextEdit* myInclude;

    protected slots:
        void onMatchModified();
        void onPreModified();
        void onMessageModified();
        void onPostModified();
        void onRepliesModified();
        void onIncludeModified();
};

#endif	/* _CRERULEPANEL_H */

