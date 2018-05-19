#pragma once

#include <QItemDelegate>
#include <QList>
#include <QStandardItemModel>

/// @brief Represents a
class DynamicItemDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  struct DelegateInfo
  {
    QVector<int> line_edit_delegates;
    QVector<int> spin_box_delegates;
    QVector<int> combo_box_delegates;
    QVector<QStandardItemModel*> combo_box_entries;
    QVector<int> combo_box_selections;
  };

  DynamicItemDelegate(DelegateInfo delegates);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&,
                        const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override;

private:
  DelegateInfo m_delegates = DelegateInfo();
};
