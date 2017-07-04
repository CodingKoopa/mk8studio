#include "IODialog.h"
#include "ui_IODialog.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>

IODialog::IODialog(Type type, QWidget* parent)
    : QDialog(parent), m_ui(new Ui::IODialog), m_current_group(0)
{
  m_ui->setupUi(this);
  QString io_string;
  switch (type)
  {
  case Type::Export:
    io_string = "Export";
    break;
  case Type::Import:
    io_string = "Import";
    break;
  case Type::Inject:
    io_string = "Inject";
    break;
  }
  setWindowTitle(io_string);
  m_ui->io_button->setText(io_string);

  connect(m_ui->io_button, &QPushButton::clicked, this, &IODialog::StartIOAction);
}

IODialog::~IODialog()
{
  delete m_ui;
}

void IODialog::AddGroup(QGroupBox* group)
{
  m_ui->options_layout->insertWidget(m_current_group, group);
  ++m_current_group;
}

void IODialog::MakeOpenFilePathConnection(QLineEdit* line_edit, QPushButton* button,
                                          const QString& filter)
{
  connect(button, &QPushButton::clicked, this, [this, line_edit, filter] {
    line_edit->setText(QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), filter));
  });
}

void IODialog::MakeSaveFilePathConnection(QLineEdit* line_edit, QPushButton* button,
                                          const QString& filter)
{
  connect(button, &QPushButton::clicked, this, [this, line_edit, filter] {
    line_edit->setText(QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), filter));
  });
}
