#include <limits>

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

#include "CustomDelegate.h"

CustomItemDelegate::CustomItemDelegate(CustomItemDelegate::DelegateGroup delegates)
    : m_delegates(delegates)
{
}

QWidget* CustomItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&,
                                          const QModelIndex& index) const
{
  if (index.column() == 0)
    return nullptr;

  else if (m_delegates.line_edit_delegates.contains(index.row()))
  {
    QLineEdit* editor = new QLineEdit(parent);
    return editor;
  }

  else if (m_delegates.spin_box_delegates.contains(index.row()))
  {
    QSpinBox* editor = new QSpinBox(parent);
    editor->setMaximum(std::numeric_limits<quint32>::max());
    return editor;
  }

  else if (m_delegates.combo_box_delegates.contains(index.row()))
  {
    if (m_delegates.combo_box_delegates.size() != m_delegates.combo_box_entries.size())
      return nullptr;

    QComboBox* editor = new QComboBox(parent);

    editor->setModel(
        m_delegates.combo_box_entries.at(m_delegates.combo_box_delegates.indexOf(index.row())));
    // select the specified index
    editor->setCurrentIndex(
        m_delegates.combo_box_selections.at(m_delegates.combo_box_delegates.indexOf(index.row())));
    return editor;
  }

  return nullptr;
}

void CustomItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (index.column() == 0)
    return;

  else if (m_delegates.line_edit_delegates.contains(index.row()))
  {
    QLineEdit* line_edit = static_cast<QLineEdit*>(editor);
    QString value = index.model()->data(index, Qt::EditRole).toString();
    line_edit->setText(value);
  }

  else if (m_delegates.spin_box_delegates.contains(index.row()))
  {
    QSpinBox* spin_box = static_cast<QSpinBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toInt();
    spin_box->setValue(value);
  }

  else if (m_delegates.combo_box_delegates.contains(index.row()))
  {
    QComboBox* combo_box = static_cast<QComboBox*>(editor);
    QString current_text = index.model()->data(index, Qt::EditRole).toString();
    int index = combo_box->findText(current_text);
    combo_box->setCurrentIndex(index);
  }
}

void CustomItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                      const QModelIndex& index) const
{
  if (m_delegates.line_edit_delegates.contains(index.row()))
  {
    QLineEdit* line_edit = static_cast<QLineEdit*>(editor);
    QString string = line_edit->text();
    model->setData(index, string, Qt::EditRole);
  }
  else if (m_delegates.spin_box_delegates.contains(index.row()))
  {
    QSpinBox* spin_box = qobject_cast<QSpinBox*>(editor);
    spin_box->interpretText();
    int value = spin_box->value();
    model->setData(index, value, Qt::EditRole);
  }
  else if (m_delegates.combo_box_delegates.contains(index.row()))
  {
    QComboBox* combo_box = qobject_cast<QComboBox*>(editor);
    model->setData(index, combo_box->currentText(), Qt::EditRole);
  }
}
