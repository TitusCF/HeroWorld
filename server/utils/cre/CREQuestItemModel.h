#ifndef _CREQUESTITEMMODEL_H
#define	_CREQUESTITEMMODEL_H

#include <QObject>
#include <QAbstractItemModel>

class Quest;

class CREQuestItemModel : public QAbstractItemModel
{
    Q_OBJECT

    public:
        CREQuestItemModel(QObject* parent);
        virtual ~CREQuestItemModel();

        Quest* quest() const;
        void setQuest(Quest* quest);

        void moveUp(int step);
        void moveDown(int step);

        virtual int columnCount(const QModelIndex& parent) const;
        virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
        virtual QModelIndex parent(const QModelIndex& index) const;
        virtual int rowCount(const QModelIndex & parent) const;
        virtual QVariant data(const QModelIndex& index, int role) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
        virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

    public slots:
        void addStep(bool);

    protected:
        Quest* myQuest;
};

#endif	/* _CREQUESTITEMMODEL_H */

