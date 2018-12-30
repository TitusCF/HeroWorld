#include <QTextEdit>
#include <qstyleditemdelegate.h>
#include "CREMultilineItemDelegate.h"
#include "CREQuestItemModel.h"
#include "Quest.h"

CREMultilineItemDelegate::CREMultilineItemDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

CREMultilineItemDelegate::~CREMultilineItemDelegate()
{
}

QWidget* CREMultilineItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    QTextEdit* edit = new QTextEdit(parent);
    edit->setAcceptRichText(false);
    return edit;
}

void CREMultilineItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (!index.isValid())
        return;
    QTextEdit* edit = qobject_cast<QTextEdit*>(editor);
    if (edit == NULL)
        return;

    edit->setPlainText(index.model()->data(index, Qt::EditRole).toString());
}

void CREMultilineItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (!index.isValid())
        return;
    QTextEdit* edit = qobject_cast<QTextEdit*>(editor);
    if (edit == NULL)
        return;
    CREQuestItemModel* qim = qobject_cast<CREQuestItemModel*>(model);
    if (qim == NULL)
        return;

    QVariant value = edit->toPlainText();
    qim->setData(index, value);
}
