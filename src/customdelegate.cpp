#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

#include "customdelegate.h"

CustomDelegate::CustomDelegate(CustomDelegate::delegateGroup_t delegates) : m_delegates(delegates)
{
#ifdef DEBUG
  console = new QTextStream(stdout, QIODevice::WriteOnly);
  *console << "Custom Delegates Debug Mode";
  console->flush();
#endif
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
    {
#ifdef DEBUG
      *console << "Combo Box Delegate mismatch! Make sure that there are an even number of "
                  "Combo Box Entries and Delegates.";
#endif
      return nullptr;
    }

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
#ifdef DEBUG
    *console << "Setting Line Edit Editor Data:  " << value;
#endif
    lineEdit->setText(value);
  }

  else if (m_delegates.spin_box_delegates.contains(index.row()))
  {
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toInt();
#ifdef DEBUG
    *console << "Setting Spin Box Editor Data: " << value;
#endif
    spinBox->setValue(value);
  }

  else if (m_delegates.combo_box_delegates.contains(index.row()))
  {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    QString currentText = index.model()->data(index, Qt::EditRole).toString();
    int index = comboBox->findText(currentText);
#ifdef DEBUG
    *console << "Setting Combo Box Editor Data: " << index;
#endif
    comboBox->setCurrentIndex(index);
  }
#ifdef DEBUG
  *console << '\n';
  console->flush();
#endif
}

void CustomDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                  const QModelIndex& index) const
{
  if (m_delegates.line_edit_delegates.contains(index.row()))
  {
    QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
    QString string = lineEdit->text();
#ifdef DEBUG
    *console << "Setting row " << index.row() << "'s internal model data: " << string;
#endif
    model->setData(index, string, Qt::EditRole);
  }
  else if (m_delegates.spin_box_delegates.contains(index.row()))
  {
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();
#ifdef DEBUG
    *console << "Setting row " << index.row() << "'s internal model data: " << value;
#endif
    model->setData(index, value, Qt::EditRole);
  }
  else if (m_delegates.combo_box_delegates.contains(index.row()))
  {
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
#ifdef DEBUG
    *console << "Setting row " << index.row()
             << "'s internal model data: " << comboBox->currentText();
#endif
    model->setData(index, comboBox->currentText(), Qt::EditRole);
  }
#ifdef DEBUG
  *console << '\n';
  console->flush();
#endif
}
