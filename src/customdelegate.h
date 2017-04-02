#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H

#include <QItemDelegate>
#include <QList>
#include <QStandardItemModel>

#ifdef CUSTOMDELEGATE_DEBUG
#include <QTextStream>
#endif

class CustomDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  struct delegateGroup_t
  {
    QList<int> line_edit_delegates;
    QList<int> spin_box_delegates;
    QList<int> combo_box_delegates;
    QList<QStandardItemModel*> combo_box_entries;
    QList<int> combo_box_selections;
  };

  CustomDelegate(delegateGroup_t delegates);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&,
                        const QModelIndex& index) const;
  void setEditorData(QWidget* editor, const QModelIndex& index) const;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

private:
  delegateGroup_t m_delegates;
#ifdef CUSTOMDELEGATE_DEBUG
  QTextStream* console;
#endif
};

#endif  // CUSTOMDELEGATE_H
