#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>

ExportDialog::ExportDialog(QWidget* parent)
    : QDialog(parent), m_ui(new Ui::ExportDialog), m_current_group(1)
{
  m_ui->setupUi(this);
  setWindowTitle("Export");
  connect(m_ui->generate_button, SIGNAL(clicked()), this, SIGNAL(StartExport()));
}

ExportDialog::~ExportDialog()
{
  delete m_ui;
}

void ExportDialog::AddGroup(QGroupBox* group)
{
  m_ui->options_layout->insertWidget(m_current_group, group);
  m_current_group++;
}

void ExportDialog::ConnectFilePathPair(QLineEdit* line_edit, QPushButton* button)
{
  QSignalMapper* mapper = new QSignalMapper(this);
  QWidget* line_edit_upcast = dynamic_cast<QWidget*>(line_edit);
  if (!line_edit_upcast)
    return;
  connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
  mapper->setMapping(button, line_edit_upcast);
  connect(mapper, SIGNAL(mapped(QWidget*)), this, SLOT(HandleFileDialogButtonClick(QWidget*)));
}

void ExportDialog::HandleFileDialogButtonClick(QWidget* line_edit)
{
  QLineEdit* line_edit_downcast = dynamic_cast<QLineEdit*>(line_edit);
  if (!line_edit_downcast)
    return;
  line_edit_downcast->setText(
      QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(),
                                   "DirectDraw Surface Texture Format (*.dds);;All Files (*.*)"));
}
