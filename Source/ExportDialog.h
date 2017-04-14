#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QComboBox>
#include <QDialog>

namespace Ui
{
class ExportDialog;
}

class ExportDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ExportDialog(QWidget* parent = 0);
  ~ExportDialog();

  void AddComboBoxOptions(QList<QList<QString>> list);

private:
  Ui::ExportDialog* m_ui;
};

#endif  // EXPORTDIALOG_H
