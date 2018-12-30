#ifndef _CREMULTILINEITEMDELEGATE_H
#define	_CREMULTILINEITEMDELEGATE_H

class CREMultilineItemDelegate : public QStyledItemDelegate
{
    public:
        CREMultilineItemDelegate(QObject* parent);
        virtual ~CREMultilineItemDelegate();

        virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
        virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

    private:
};

#endif	/* _CREMULTILINEITEMDELEGATE_H */

