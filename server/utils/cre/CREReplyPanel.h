#ifndef _CREREPLYPANEL_H
#define	_CREREPLYPANEL_H

#include <QWidget>

class QTreeWidget;
class QLineEdit;
class QComboBox;
class QTreeWidgetItem;

/**
 * This panel is the 'replies' subpanel in the messages panel.
 */
class CREReplyPanel : public QWidget
{
    Q_OBJECT

    public:
        CREReplyPanel(QWidget* parent);
        virtual ~CREReplyPanel();

        void setData(const QList<QStringList>& data);
        QList<QStringList> getData();

    signals:
        /** Emitted when the data this panel manages changes .*/
        void dataModified();

    private:
        /** The data this panel is working on. */
        QList<QStringList> myData;
        /** Display for the replies. */
        QTreeWidget* myReplies;
        /** Zone to edit the reply keyword. */
        QLineEdit* myText;
        /** Zone to edit the message the NPC will say. */
        QLineEdit* myMessage;
        /** Available reply type. */
        QComboBox* myType;

        /**
         * Fill a line of myReplies with the data.
         * @param item line to fill.
         * @param data contents to put in the line.
         */
        void setText(QTreeWidgetItem* item, QStringList data);
        /**
         * Update the currently being edited reply with data in the various fields.
         */
        void updateItem();

    private slots:
        void currentReplyChanged(QTreeWidgetItem*, QTreeWidgetItem*);
        void onAddItem(bool);
        void onDeleteItem(bool);
        void onTextChanged(const QString&);
        void onTypeChanged(int);
};

#endif	/* _CREREPLYPANEL_H */
