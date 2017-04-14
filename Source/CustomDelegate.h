#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H

#include <QItemDelegate>
#include <QList>
#include <QStandardItemModel>

class CustomDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  struct DelegateGroup
  {
    QList<int> line_edit_delegates;
    QList<int> spin_box_delegates;
    QList<int> combo_box_delegates;
    QList<QStandardItemModel*> combo_box_entries;
    QList<int> combo_box_selections;
  };

  CustomDelegate(DelegateGroup delegates);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&,
                        const QModelIndex& index) const;
  void setEditorData(QWidget* editor, const QModelIndex& index) const;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

private:
  DelegateGroup m_delegates;
};

#endif  // CUSTOMDELEGATE_H
