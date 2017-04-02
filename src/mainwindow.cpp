// TODO: some of these might not be necessary
#include <QAction>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QStandardItemModel>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "mainwindow.h"

#include "customdelegate.h"
#include "filebase.h"
#include "nodes/bfresnode.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
  m_ui->setupUi(this);

  connect(m_ui->action_open, SIGNAL(triggered()), this, SLOT(OpenFile()));
  connect(m_ui->action_save, SIGNAL(triggered()), this, SLOT(SaveFile()));

  // Splitter for the left side of the UI
  m_file_tree_attributes_splitter = new QSplitter();
  m_file_tree_attributes_splitter->setOrientation(Qt::Vertical);
  m_left_right_splitter = new QSplitter();
  m_left_right_splitter->addWidget(m_file_tree_attributes_splitter);

  // m_left_right_splitter->show();

  m_ui->statusbar->showMessage("Ready.");

  // temp
  OpenFile();
}

MainWindow::~MainWindow()
{
  delete m_ui;
}

void MainWindow::OpenFile()
{
  FileBase* file;

  file = new FileBase("/home/kyle/External/Nokonoko.bfres");
  //  file = new FileBase(
  //      QFileDialog::getOpenFileName(this, tr("Open File"), tr("BFRES Model (*.bfres)")));
  if (!file->GetCanRead())
  {
    // TODO: throw error message the right way
    QMessageBox::critical(
        this, "Error",
        QString("%0 could not be opened for reading.").arg("/home/kyle/External/Nokonoko.bfres"));
    return;
  }

  m_current_file_node = new BFRESNode(BFRES(file), this);

  // Make the connections for the BFRES node and any children.
  connect(m_current_file_node, SIGNAL(ConnectNode(Node*)), this, SLOT(MakeNodeConnections(Node*)));

  if (m_current_file_node->LoadFileTreeArea() != RESULT_SUCCESS)
    return;
  if (m_current_file_node->LoadAttributeArea() != RESULT_SUCCESS)
    return;

  m_ui->welcome_widget->hide();
  setCentralWidget(m_left_right_splitter);

  // temp
  SaveFile();
}

void MainWindow::SaveFile()
{
  m_current_file_node->SaveFile();
}

void MainWindow::MakeNodeConnections(Node* node)
{
  connect(node, SIGNAL(NewFileTreeArea(QScrollArea*)), this,
          SLOT(UpdateFileTreeContainer(QScrollArea*)));
  connect(node, SIGNAL(NewAttributesArea(QScrollArea*)), this,
          SLOT(UpdateSectionsContainer(QScrollArea*)));
  connect(node, SIGNAL(NewMainWidget(QWidget*)), this, SLOT(UpdateMainWidget(QWidget*)));
  connect(node, SIGNAL(NewStatus(ResultCode, QString)), this,
          SLOT(UpdateStatus(ResultCode, QString)));
}

void MainWindow::UpdateFileTreeContainer(QScrollArea* area)
{
  if (m_file_tree_attributes_splitter->widget(0))
  {
    m_file_tree_attributes_splitter->widget(1)->hide();
    delete m_file_tree_attributes_splitter->widget(1);
  }
  m_file_tree_attributes_splitter->addWidget(area);
}

void MainWindow::UpdateSectionsContainer(QScrollArea* area)
{
  if (m_file_tree_attributes_splitter->widget(1))
  {
    m_file_tree_attributes_splitter->widget(1)->hide();
    delete m_file_tree_attributes_splitter->widget(1);
  }
  m_file_tree_attributes_splitter->addWidget(area);
}

void MainWindow::UpdateMainWidget(QWidget* widget)
{
  if (m_left_right_splitter->widget(1))
  {
    // TODO: there's probably a better way of replacing this widget
    m_left_right_splitter->widget(1)->hide();
    delete m_left_right_splitter->widget(1);
  }
  m_left_right_splitter->addWidget(widget);
}

void MainWindow::UpdateStatus(ResultCode status, QString message)
{
  switch (status)
  {
  case RESULT_SUCCESS:
    m_ui->statusbar->showMessage("Success.");
    break;
  case RESULT_STATUS_BAR_UPDATE:
    m_ui->statusbar->showMessage(message);
    break;
  case RESULT_BFRES_HEADER_SIZE_ERROR:
    QMessageBox::critical(this, "Error", "Failed to read BFRES header. This may be due to either a "
                                         "corrupt file or a bug in the code.");
    break;
  case RESULT_BFRES_ENDIANNESS:
    QMessageBox::critical(this, "Error",
                          "Invalid BFRES endianness. Must be either 0xFEFF or 0xFFFE.");
    break;
  case RESULT_UNSUPPORTED_FILE_FORMAT:
    QMessageBox::critical(this, "Error", "Unsupported texture format.");
    break;
  }

  // If there was an error.
  if (status != RESULT_SUCCESS && status != RESULT_STATUS_BAR_UPDATE)
    m_ui->statusbar->showMessage("Failed.");
}
