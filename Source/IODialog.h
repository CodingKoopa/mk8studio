#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>

namespace Ui
{
class IODialog;
}

class IODialog : public QDialog
{
  Q_OBJECT

public:
  enum class Type
  {
    Export,
    Import,
    Inject
  };

  explicit IODialog(Type type, QWidget* parent = 0);
  ~IODialog();
  void AddGroup(QGroupBox* group);
  void MakeOpenFilePathConnection(QLineEdit* line_edit, QPushButton* button, QString filter);
  void MakeSaveFilePathConnection(QLineEdit* line_edit, QPushButton* button, QString filter);

private:
  Ui::IODialog* m_ui;
  quint32 m_current_group;

signals:
  void StartIOAction();
};

#endif  // EXPORTDIALOG_H
