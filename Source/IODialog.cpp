#include "IODialog.h"

#include <functional>

#include <QFileDialog>
#include <QFont>
#include <QFontMetrics>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSignalMapper>

IODialog::IODialog(Type type, const QString& last_path_key, const QString& name,
                   const QString& path_filter, QStandardPaths::StandardLocation default_dir,
                   const QString& default_extention, QWidget* parent)
    : QDialog(parent), m_options_layout(new QVBoxLayout)
{
  // As the first UI element, add the path editor.

  // Get the last used path.
  QString path;
  QSettings settings;
  settings.beginGroup("file_paths");
  QString last_path = settings.value(last_path_key).toString();
  settings.endGroup();
  // Try to guess what path the user might want to save to, based off of the format type, format
  // name, and last path saved to.
  // If a last path was found.
  if (!last_path.isEmpty())
  {
    QFileInfo file_info(last_path);
    // Take the directory of the last used path.
    path = file_info.dir().path() + QDir::separator() + name + '.' + file_info.suffix();
  }
  // If no last path was found.
  else
  {
    path = QStandardPaths::standardLocations(default_dir).first();
    // If the caller didn't speficy a default directory to fallback to.
    if (path.isEmpty())
      path = QDir::homePath();
    path += QDir::separator() + name;
    if (!default_extention.isEmpty())
      path += '.' + default_extention;
  }
  // With the guessed path, append to it the internal file name, and the
  m_path_line_edit = new QLineEdit(path);
  ResizeLineEditToText(m_path_line_edit);

  QPushButton* path_button = new QPushButton("...");
  switch (type)
  {
  case Type::Export:
    connect(path_button, &QPushButton::clicked, this, [this, path_filter, path] {
      m_path_line_edit->setText(QFileDialog::getSaveFileName(this, "Save File", path, path_filter));
      ResizeLineEditToText(m_path_line_edit);
    });
    break;
  case Type::Import:
  case Type::Inject:
    connect(path_button, &QPushButton::clicked, this, [this, path_filter, path] {
      m_path_line_edit->setText(QFileDialog::getOpenFileName(this, "Open File", path, path_filter));
      ResizeLineEditToText(m_path_line_edit);
    });
    break;
  }

  QHBoxLayout* path_layout = new QHBoxLayout;
  path_layout->addWidget(m_path_line_edit);
  path_layout->addWidget(path_button);

  // As the last UI element, add the action button.

  QString action_string;
  switch (type)
  {
  case Type::Export:
    action_string = "Export";
    break;
  case Type::Import:
    action_string = "Import";
    break;
  case Type::Inject:
    action_string = "Inject";
    break;
  }
  QPushButton* action_button = new QPushButton(action_string);
  connect(action_button, &QPushButton::clicked, this,
          [this] { emit StartAction(m_path_line_edit->text()); });

  // Add the UI elements to the dialog.

  m_options_layout->addLayout(path_layout);
  m_options_layout->addWidget(action_button);
  setLayout(m_options_layout);

  // Set the dialog title.

  setWindowTitle(action_string);
}

void IODialog::AddGroup(QGroupBox* group)
{
  m_options_layout->insertWidget(m_current_group, group);
  ++m_current_group;
}

void IODialog::ResizeLineEditToText(QLineEdit* line_edit)
{
  QFont font;
  QFontMetrics font_metrics(font);
  line_edit->setMinimumWidth(font_metrics.width(line_edit->text()));
  line_edit->adjustSize();
}
