#include "CREQuestItemModel.h"
#include "Quest.h"

CREQuestItemModel::CREQuestItemModel(QObject* parent) : QAbstractItemModel(parent)
{
    myQuest = NULL;
}

CREQuestItemModel::~CREQuestItemModel() {
}

Quest* CREQuestItemModel::quest() const
{
    return myQuest;
}

void CREQuestItemModel::setQuest(Quest* quest)
{
    myQuest = quest;
    reset();
}

int CREQuestItemModel::columnCount(const QModelIndex& parent) const
{
    if (myQuest == NULL)
        return 0;

    if (parent.isValid())
        return 0;

    return 4;
}

QModelIndex CREQuestItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex CREQuestItemModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int CREQuestItemModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;

    if (myQuest == NULL)
        return 0;

    return myQuest->steps().size();
}

QVariant CREQuestItemModel::data(const QModelIndex& index, int role) const
{
    if (myQuest == NULL || !index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole && (role != Qt::CheckStateRole || index.column() != 2))
        return QVariant();

    Q_ASSERT(index.row() < myQuest->steps().size());
    const QuestStep* step = myQuest->steps()[index.row()];

    switch(index.column())
    {
        case 0:
            return step->step();

        case 1:
            return step->description();

        case 2:
            if (role == Qt::DisplayRole)
                return QVariant();
            return step->isCompletion() ? Qt::Checked : Qt::Unchecked;

        case 3:
            /*if (role == Qt::EditRole)
                return step->setWhen();*/
            return step->setWhen().join("\n");
    }

    return QVariant();;
}

QVariant CREQuestItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch(section)
    {
        case 0:
            return tr("step");
        case 1:
            return tr("description");
        case 2:
            return tr("end?");
        case 3:
            return tr("set when");
    }

    return QVariant();
}

Qt::ItemFlags CREQuestItemModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    if (index.column() == 2)
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;

    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool CREQuestItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || myQuest == NULL || myQuest->steps().size() <= index.row())
        return false;

    QuestStep* step = myQuest->steps()[index.row()];

    if (role != Qt::EditRole && index.column() != 2)
        return false;

    if (index.column() == 0)
        step->setStep(value.toInt());
    else if (index.column() == 1)
        step->setDescription(value.toString());
    else if (index.column() == 2)
        step->setCompletion(value == Qt::Checked);
    else if (index.column() == 3)
        step->setWhen() = value.toString().split("\n");

    emit dataChanged(index, index);
    myQuest->setModified(true);

    return true;
}

void CREQuestItemModel::addStep(bool)
{
    if (myQuest == NULL)
        return;

    beginInsertRows(QModelIndex(), myQuest->steps().size(), myQuest->steps().size());

    myQuest->steps().append(new QuestStep());
    myQuest->setModified(true);

    endInsertRows();
}

bool CREQuestItemModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (myQuest == NULL || parent.isValid() || count != 1)
        return false;

    if (row < 0 || row >= myQuest->steps().size())
        return false;

    beginRemoveRows(parent, row, row);
    delete myQuest->steps().takeAt(row);
    endRemoveRows();

    return true;
}

void CREQuestItemModel::moveUp(int step)
{
    if (step < 1)
        return;

    beginMoveRows(QModelIndex(), step, step, QModelIndex(), step - 1);
    QuestStep* s = myQuest->steps()[step];
    myQuest->steps()[step] = myQuest->steps()[step - 1];
    myQuest->steps()[step - 1] = s;
    endMoveRows();
    myQuest->setModified(true);
}

void CREQuestItemModel::moveDown(int step)
{
    if (step >= myQuest->steps().size() - 1)
        return;

    beginMoveRows(QModelIndex(), step + 1, step + 1, QModelIndex(), step);
    QuestStep* s = myQuest->steps()[step];
    myQuest->steps()[step] = myQuest->steps()[step + 1];
    myQuest->steps()[step + 1] = s;
    endMoveRows();
    myQuest->setModified(true);
}
