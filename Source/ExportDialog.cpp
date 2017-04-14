#include "ExportDialog.h"
#include "ui_exportdialog.h"

ExportDialog::ExportDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::ExportDialog)
{
  m_ui->setupUi(this);
  m_ui->main_widget->hide();
}

ExportDialog::~ExportDialog()
{
  delete m_ui;
}

void ExportDialog::AddComboBoxOptions(QList<QList<QString>> list)
{
}
