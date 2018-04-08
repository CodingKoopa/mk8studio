#include "MainWindow.h"

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

#include "CustomDelegate.h"
#include "Nodes/Archives/BFRESNode.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
  m_ui->setupUi(this);

  m_vertical_splitter = new QSplitter;
  m_vertical_splitter->setOrientation(Qt::Vertical);

  m_horizontal_splitter = new QSplitter;
  m_horizontal_splitter->addWidget(m_vertical_splitter);

  setWindowTitle("Mario Kart 8 Studio");
  connect(m_ui->action_open, SIGNAL(triggered()), this, SLOT(OpenFile()));

  m_ui->welcome_widget->show();

  m_ui->statusbar->showMessage("Ready.");

  QCoreApplication::setOrganizationName("mk8mc");
  QCoreApplication::setApplicationName("mk8studio");
  QSettings settings;
  settings.beginGroup("file_paths");
  if (!settings.value("last_main_file").toString().isEmpty())
    OpenFile(settings.value("last_main_file").toString());
  settings.endGroup();
}

MainWindow::~MainWindow()
{
  delete m_ui;
}

void MainWindow::OpenFile(const QString& path)
{
  if (path.isEmpty())
    m_file = std::make_shared<File>(
        QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), "BFRES Model (*.bfres)"));
  else
    m_file = std::make_shared<File>(path);

  if (!m_file->GetCanRead())
  {
    emit UpdateStatus(ResultCode::FileNotFound, path);
    return;
  }

  m_root_node = std::make_unique<BFRESNode>(std::make_shared<BFRES>(m_file), this);
  m_root_node->LoadAttributeArea();

  // Make the connections for the BFRES node and any children.
  connect(m_root_node.get(), &Node::ConnectNode, this, &MainWindow::MakeNodeConnections);

  if (m_root_node->LoadFileTreeArea() != ResultCode::Success ||
      m_root_node->LoadAttributeArea() != ResultCode::Success)
    return;

  setCentralWidget(m_horizontal_splitter);
}

void MainWindow::MakeNodeConnections(Node* node)
{
  connect(node, &Node::NewFileTreeArea, this, &MainWindow::UpdateFileTreeContainer);
  connect(node, &Node::NewAttributeArea, this, &MainWindow::UpdateAttributeContainer);
  connect(node, &Node::NewMainWidget, this, &MainWindow::UpdateMainWidget);
  connect(node, &Node::NewStatus, this, &MainWindow::UpdateStatus);
}

void MainWindow::UpdateFileTreeContainer(QScrollArea* area)
{
  if (m_vertical_splitter->widget(0))
    delete m_vertical_splitter->widget(0);
  m_vertical_splitter->insertWidget(0, area);
}

void MainWindow::UpdateAttributeContainer(QScrollArea* area)
{
  if (m_vertical_splitter->widget(1))
    delete m_vertical_splitter->widget(1);
  m_vertical_splitter->insertWidget(1, area);
}

void MainWindow::UpdateMainWidget(QWidget* widget)
{
  if (m_horizontal_splitter->widget(1))
    delete m_horizontal_splitter->widget(1);
  m_horizontal_splitter->addWidget(widget);
}

void MainWindow::UpdateStatus(ResultCode status, const QString& details)
{
  switch (status)
  {
  case ResultCode::Success:
    m_ui->statusbar->showMessage("Success.");
    return;
  case ResultCode::UpdateStatusBar:
    m_ui->statusbar->showMessage(details);
    return;

  case ResultCode::NotAvailable:
    QMessageBox::critical(this, "Error",
                          QString("The requested widget isn't available for this file type."));
    break;
  case ResultCode::FileNotFound:
    QMessageBox::critical(this, "Error",
                          QString("Couldn't open file %0 for reading.").arg(details));
    break;
  case ResultCode::NoBytesWritten:
    QMessageBox::critical(
        this, "Error",
        QString("No bytes were written to the file. This may be due to it not being found, or this "
                "application not having sufficient permissions."));
    break;
  case ResultCode::IncorrectHeaderSize:
    QMessageBox::critical(
        this, "Error",
        "Header size didn't match. This may be due to either a corrupt file or a bug in the code.");
    break;
  case ResultCode::IncorrectBFRESEndianness:
    QMessageBox::critical(this, "Error",
                          "Invalid BFRES endianness. Must be either 0xFEFF or 0xFFFE.");
    break;
  case ResultCode::UnsupportedTextureFormat:
    QMessageBox::critical(this, "Error", "Unsupported FTEX texture format.");
    break;
  case ResultCode::UnsupportedFTEXComponent:
    QMessageBox::critical(this, "Error", "Unsupported FTEX component.");
    break;
  case ResultCode::UnsupportedAttributeFormat:
    QMessageBox::critical(this, "Error", "Unsupported FVTX attribute format.");
    break;
  case ResultCode::ImportantUnsupportedFileFormat:
    QMessageBox::critical(this, "Error",
                          "This texture has anti-aliasing, thickness, or is 3D, which is "
                          "unsupported. Please send this file to Koopa ASAP!");
    break;
  }

  m_ui->statusbar->showMessage("Failed.");
}
