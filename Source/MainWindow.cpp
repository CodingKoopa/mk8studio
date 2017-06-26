// TODO: some of these might not be necessary
#include <QAction>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSplitter>
#include <QStandardItemModel>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "MainWindow.h"

#include "CustomDelegate.h"
#include "FileBase.h"
#include "Nodes/BFRESNode.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
  m_ui->setupUi(this);

  QCoreApplication::setOrganizationName("Mario Kart 8 Modding Central");
  QCoreApplication::setApplicationName("Mario Kart 8 Studio");
  setWindowTitle("Mario Kart 8 Studio");

  QSettings settings;

  connect(m_ui->action_open, SIGNAL(triggered()), this, SLOT(OpenFile()));

  // Splitter for the left side of the UI
  m_file_tree_attributes_splitter = new QSplitter();
  m_file_tree_attributes_splitter->setOrientation(Qt::Vertical);
  m_left_right_splitter = new QSplitter();
  m_left_right_splitter->addWidget(m_file_tree_attributes_splitter);

  settings.beginGroup("file_paths");
  if (!settings.value("last_main_file").toString().isEmpty())
    OpenFile(settings.value("last_main_file").toString());
  settings.endGroup();

  m_ui->welcome_widget->show();

  m_ui->statusbar->showMessage("Ready.");
}

MainWindow::~MainWindow()
{
  delete m_ui;
}

void MainWindow::OpenFile(QString path)
{
  if (path.isEmpty())
    path =
        QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), "BFRES Model (*.bfres)");

  FileBase* file = new FileBase(path);

  //  file = new FileBase(path);
  if (!file->GetCanRead())
  {
    emit UpdateStatus(ResultCode::FileNotFound, path);
    return;
  }

  // Deallocated by BFRESNode's dtor.
  BFRES* bfres = new BFRES(file);
  m_current_file_node = new BFRESNode(bfres, this);

  // Make the connections for the BFRES node and any children.
  connect(m_current_file_node, SIGNAL(ConnectNode(Node*)), this, SLOT(MakeNodeConnections(Node*)));

  if (m_current_file_node->LoadFileTreeArea() != ResultCode::Success)
    return;
  if (m_current_file_node->LoadAttributeArea() != ResultCode::Success)
    return;

  setCentralWidget(m_left_right_splitter);
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
    delete m_file_tree_attributes_splitter->widget(0);
  }
  m_file_tree_attributes_splitter->insertWidget(0, area);
}

void MainWindow::UpdateSectionsContainer(QScrollArea* area)
{
  if (m_file_tree_attributes_splitter->widget(1))
  {
    area->resize(m_file_tree_attributes_splitter->widget(1)->size());
    delete m_file_tree_attributes_splitter->widget(1);
  }
  m_file_tree_attributes_splitter->insertWidget(1, area);
}

void MainWindow::UpdateMainWidget(QWidget* widget)
{
  if (m_left_right_splitter->widget(1))
  {
    widget->resize(m_left_right_splitter->widget(1)->size());
    delete m_left_right_splitter->widget(1);
  }
  m_left_right_splitter->addWidget(widget);
}

void MainWindow::UpdateStatus(ResultCode status, QString details)
{
  switch (status)
  {
  case ResultCode::FileNotFound:
    QMessageBox::critical(this, "Error",
                          QString("Couldn't open file %0 for reading.").arg(details));
    break;
  case ResultCode::NoBytesWritten:
    QMessageBox::critical(
        this, "Error", QString("No bytes were written to the file. This may be due to it not being "
                               "found, or this application not having sufficient permissions."));
    break;
  case ResultCode::IncorrectHeaderSize:
    QMessageBox::critical(this, "Error", "Header size didn't match. This may be due to either a "
                                         "corrupt file or a bug in the code.");
    break;
  case ResultCode::IncorrectBFRESEndianness:
    QMessageBox::critical(this, "Error",
                          "Invalid BFRES endianness. Must be either 0xFEFF or 0xFFFE.");
    break;
  case ResultCode::UnsupportedFileFormat:
    QMessageBox::critical(this, "Error", "Unsupported texture format.");
    break;
  case ResultCode::ImportantUnsupportedFileFormat:
    QMessageBox::critical(this, "Error",
                          "This texture has anti-aliasing, thickness, or is 3D, which is "
                          "unsupported. Please send this file to Koopa ASAP!");
    break;
  default:
    break;
  }

  switch (status)
  {
  case ResultCode::Success:
    m_ui->statusbar->showMessage("Success.");
    break;
  case ResultCode::UpdateStatusBar:
    m_ui->statusbar->showMessage(details);
    break;
  default:
    m_ui->statusbar->showMessage("Failed.");
    break;
  }
}
