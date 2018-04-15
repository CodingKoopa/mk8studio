#pragma once

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace Ui
{
class IODialog;
}

/// @brief Represents a dialog for input/output operations.
///
/// This class is an interface that allows the creation of a dialog, and the adding of groups of
/// options. From a method like FTEXNode::HandleExportActionClick, usage goas as follows:
/// - Instantiate an IODialog object, specifying the IODialog::Type.
/// - Create any desired UI elements such as a format QComboBox.
/// - Add the UI elements to one or more QGroupBoxs.
/// - Submit the groups via IODialog::AddGroup.
/// - Connect the IODialog::StartAction signal to a slot that will execute the process.
/// - Show this dialog.
class IODialog : public QDialog
{
  Q_OBJECT

public:
  /// The operation type of the dialog. Used for the button label.
  enum class Type
  {
    /// Exporting something.
    Export,
    /// Importing something.
    Import,
    /// Injecting something.
    Inject
  };

  /// Initializes a new instance of the IODialog class.
  ///
  /// @param  type              The type of the dialog.
  /// @param  last_path_key     The settings key for the last path used.
  /// @param  name              The name of the file to suggest.
  /// @param  path_filter       The filter for the file dialog, be it opening or saving.
  /// @param  default_dir_path  The default directory to save to.
  /// @param  default_extention The default extention to use.
  /// @param  parent            The pointer to the parent widget.
  explicit IODialog(
      Type type, const QString& last_path_key, const QString& name, const QString& path_filter,
      QStandardPaths::StandardLocation default_dir = QStandardPaths::DocumentsLocation,
      const QString& default_extention = QString(), QWidget* parent = 0);

  /// Adds a group of options after the path editor, but before the action button.
  ///
  /// @param  group   The group to add.
  void AddGroup(QGroupBox* group);

private:
  /// Resizes a line edit widget to fit most of the text.
  ///
  /// @param  line_edit   The pointer to the line edit widget.
  void ResizeLineEditToText(QLineEdit* line_edit);

  /// The central layout of options. Used to insert widgets into.
  QVBoxLayout* m_options_layout = nullptr;
  /// The line edit widget for the path. Used to access the current path from anywhere..
  QLineEdit* m_path_line_edit = nullptr;

  /// The current index for IODialog::AddGroup to insert the group into.
  quint32 m_current_group = 1;

signals:
  /// Signal emitted when the user has pressed the action button.
  ///
  /// @param  path    The path to the file to operate on.
  void StartAction(const QString& path);
};
