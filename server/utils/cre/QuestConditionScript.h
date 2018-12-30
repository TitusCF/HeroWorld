#ifndef _QUESTCONDITIONSCRIPT_H
#define	_QUESTCONDITIONSCRIPT_H

#include <QObject>

class QuestConditionScript : public QObject
{
    Q_OBJECT

    public:
        QuestConditionScript(const QString& name, const QString& comment);
        virtual ~QuestConditionScript();

        const QString& name() const;
        const QString& comment() const;

    private:
        QString myName;
        QString myComment;
};

#endif	/* _QUESTCONDITIONSCRIPT_H */

