#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QSplitter>
#include <QStandardItem>
#include <QTableView>
#include <QVBoxLayout>

#include "Common.h"
#include "Formats/BFRES.h"
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

  Node* m_current_file_node;

private slots:
  void MakeNodeConnections(Node* node);
  void UpdateFileTreeContainer(QScrollArea* area);
  void UpdateSectionsContainer(QScrollArea* area);
  void UpdateMainWidget(QWidget* widget);
  void UpdateStatus(ResultCode status = RESULT_SUCCESS, QString message = QString());

  void OpenFile();
};

#endif  // MAINWINDOW_H
