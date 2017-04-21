#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>

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
  void AddGroup(QGroupBox* group);
  void ConnectFilePathPair(QLineEdit* line_edit, QPushButton* button);

private:
  Ui::ExportDialog* m_ui;
  int m_current_group;

private slots:
  void HandleFileDialogButtonClick(QWidget* line_edit);
signals:
  void StartExport();
};

#endif  // EXPORTDIALOG_H
