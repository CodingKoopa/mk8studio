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
    QVector<int> line_edit_delegates;
    QVector<int> spin_box_delegates;
    QVector<int> combo_box_delegates;
    QVector<QStandardItemModel*> combo_box_entries;
    QVector<int> combo_box_selections;
  };

  CustomItemDelegate(DelegateGroup delegates);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&,
                        const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override;

private:
  DelegateGroup m_delegates = DelegateGroup();
};
