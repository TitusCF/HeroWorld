#ifndef _CREPREPOSTPANEL_H
#define	_CREPREPOSTPANEL_H

#include <QWidget>
#include <QObject>
#include <QtGui>

#include "MessageManager.h"

class QListWidget;
class QComboBox;
class QLineEdit;
class QuestConditionScript;
class QuestManager;

/**
 * Base class for a pre- or post- panel displaying script arguments.
 */
class CRESubItemWidget : public QWidget
{
    Q_OBJECT

    public:
        CRESubItemWidget(QWidget* parent) : QWidget(parent) { };

        virtual void setData(const QStringList& data) = 0;

    signals:
        void dataModified(const QStringList& data);
};

/**
 * Post-condition panel displaying a connection (number).
 */
class CRESubItemConnection : public CRESubItemWidget
{
    Q_OBJECT

    public:
        CRESubItemConnection(QWidget* parent);

        virtual void setData(const QStringList& data);

    private:
        QLineEdit* myEdit;
        QLabel* myWarning;

        void showWarning(const QString& warning);

    private slots:
        void editChanged(const QString& text);
};

/**
 * Pre- or post- conditions panel displaying a quest step
 */
class CRESubItemQuest : public CRESubItemWidget
{
    Q_OBJECT

    public:
        CRESubItemQuest(bool isPre, const QuestManager* quests, QWidget* parent);

        virtual void setData(const QStringList& data);

    private:
        const QuestManager* myQuests;
        bool myIsPre;
        /** List of quests. */
        QComboBox* myQuestList;
        /** Steps of the current quest for new step (post-) or at/frop step (pre-). */
        QComboBox* myFirstStep;
        /** Steps of the current quest for up to step (pre-). */
        QComboBox* mySecondStep;
        QRadioButton* myAtStep;
        QRadioButton* myFromStep;
        QRadioButton* myStepRange;
        bool myInit;

        void fillQuestSteps();
        void updateData();

    private slots:
        void selectedQuestChanged(int index);
        void checkToggled(bool checked);
        void selectedStepChanged(int index);
};

/**
 * Pre- or post- conditions panel displaying a token, either as read or write.
 */
class CRESubItemToken : public CRESubItemWidget
{
    Q_OBJECT

    public:
        CRESubItemToken(bool isPre, QWidget* parent);

        virtual void setData(const QStringList& data);

    private:
        QLineEdit* myToken;
        QLineEdit* myValue;
        QTextEdit* myValues;

        void updateData();

    private slots:
        void tokenChanged(const QString&);
        void valuesChanged();
};

/**
 * Pre- or post- panel displaying script arguments as a string list.
 */
class CRESubItemList : public CRESubItemWidget
{
    Q_OBJECT

    public:
        CRESubItemList(QWidget* parent);
        void setData(const QStringList& data);

    private:
        /** For one condition, arguments to the script. */
        QListWidget* mySubItems;
        /** Argument edit zone. */
        QLineEdit* myItemEdit;
        /** Current arguments. */
        QStringList myData;

    private slots:
        void currentSubItemChanged(int);
        void subItemChanged(const QString& text);
        void onAddSubItem(bool);
        void onDeleteSubItem(bool);
};

/**
 * This panel is the 'pre' or 'post' subpanel in the messages panel.
 */
class CREPrePostPanel : public QWidget
{
    Q_OBJECT

    public:
        /**
         * Standard constructor.
         * @param isPre true if displaying preconditions, false for postconditions.
         * @param scripts available script types for the conditions.
         * @param parent ancestor of this panel.
         */
        CREPrePostPanel(bool isPre, const QList<QuestConditionScript*> scripts, const QuestManager* quests, QWidget* parent);
        virtual ~CREPrePostPanel();

        QList<QStringList> getData();
        void setData(const QList<QStringList> data);

    signals:
        /** Emitted when the data this panel manages was changed. */
        void dataModified();

    private:
        /** Pre- or post- conditions we're working on. */
        QList<QStringList> myData;
        /** The first item of each condition. */
        QListWidget* myItems;
        /** Available conditions types. */
        QComboBox* myChoices;
        /** Matching between index of myChoices and the variable subpanels. */
        QList<CRESubItemWidget*> mySubWidgets;
        /** Arguments panels, only one visible based on the choice. */
        QStackedWidget* mySubItemsStack;

        /**
         * Creates a CRESubItemWidget for the specified script.
         * @param isPre true if pre-condition, false for post-condition.
         * @param script the script to create the display for.
         * @param quests available quests, for specific panel.
         * @return specialised CRESubItemWidget if available, CRESubItemList else.
         */
        CRESubItemWidget* createSubItemWidget(bool isPre, const QuestConditionScript* script, const QuestManager* quests);

    private slots:
        void onAddItem(bool);
        void onDeleteItem(bool);
        void currentItemChanged(int index);
        void currentChoiceChanged(int index);
        void subItemChanged(const QStringList& data);
};

#endif	/* _CREPREPOSTPANEL_H */

