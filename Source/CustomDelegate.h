#pragma once

#include <QItemDelegate>
#include <QList>
#include <QStandardItemModel>

class CustomItemDelegate : public QItemDelegate
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

  CustomItemDelegate(DelegateGroup delegates);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&,
                        const QModelIndex& index) const;
  void setEditorData(QWidget* editor, const QModelIndex& index) const;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

private:
  DelegateGroup m_delegates;
};
