#pragma once

#include <QMainWindow>
#include <QScrollArea>
#include <QSplitter>
#include <QStandardItem>
#include <QTableView>
#include <QVBoxLayout>

#include "Common.h"
#include "File.h"
#include "Nodes/Node.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow* m_ui;

  QSplitter* m_file_tree_attributes_splitter = nullptr;
  QSplitter* m_left_right_splitter = nullptr;
  File* m_file = nullptr;

  Node* m_current_file_node = nullptr;

private slots:
  void MakeNodeConnections(Node* node);
  void UpdateFileTreeContainer(QScrollArea* area);
  void UpdateSectionsContainer(QScrollArea* area);
  void UpdateMainWidget(QWidget* widget);
  void UpdateStatus(ResultCode status = ResultCode::Success, const QString& details = QString());

  void OpenFile(const QString& path = QString());
};
