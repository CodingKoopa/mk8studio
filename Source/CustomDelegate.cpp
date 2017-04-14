#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

#include "CustomDelegate.h"

CustomDelegate::CustomDelegate(CustomDelegate::DelegateGroup delegates) : m_delegates(delegates)
{
}

QWidget* CustomDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&,
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
    // TODO: get an actual value for this?
    editor->setMaximum(1000000000);
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
    // editor->setCurrentIndex(comboBoxSelections.at(m_delegates.combo_box_delegates.indexOf(index.row())));
    return editor;
  }

  return nullptr;
}

void CustomDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (index.column() == 0)
    return;

  else if (m_delegates.line_edit_delegates.contains(index.row()))
  {
    QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
    QString value = index.model()->data(index, Qt::EditRole).toString();
    lineEdit->setText(value);
  }

  else if (m_delegates.spin_box_delegates.contains(index.row()))
  {
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toInt();
    spinBox->setValue(value);
  }

  else if (m_delegates.combo_box_delegates.contains(index.row()))
  {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    QString currentText = index.model()->data(index, Qt::EditRole).toString();
    int index = comboBox->findText(currentText);
    comboBox->setCurrentIndex(index);
  }
}

void CustomDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                  const QModelIndex& index) const
{
  if (m_delegates.line_edit_delegates.contains(index.row()))
  {
    QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
    QString string = lineEdit->text();
    model->setData(index, string, Qt::EditRole);
  }
  else if (m_delegates.spin_box_delegates.contains(index.row()))
  {
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
  }
  else if (m_delegates.combo_box_delegates.contains(index.row()))
  {
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
  }
}
