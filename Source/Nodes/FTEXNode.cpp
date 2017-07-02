#include "FTEXNode.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSettings>

#include "Common.h"
#include "CustomStandardItem.h"
#include "IODialog.h"

FTEXNode::FTEXNode(const FTEX& ftex, QObject* parent) : Node(parent), m_ftex(ftex)
{
  m_context_menu = new QMenu;

  QAction* action_export = new QAction("Export", this);
  connect(action_export, SIGNAL(triggered()), this, SLOT(HandleExportActionClick()));
  m_context_menu->addAction(action_export);

  QAction* action_inject = new QAction("Inject", this);
  connect(action_inject, SIGNAL(triggered()), this, SLOT(HandleInjectActionClick()));
  m_context_menu->addAction(action_inject);
}

QStandardItem* FTEXNode::MakeItem()
{
  QStandardItem* item = new QStandardItem();
  item->setData(QString(m_ftex.GetName() + " (FTEX)"), Qt::DisplayRole);
  item->setData(QVariant::fromValue(static_cast<Node*>(this)), Qt::UserRole + 1);
  return item;
}

ResultCode FTEXNode::LoadAttributeArea()
{
  QStandardItemModel* header_attributes_model = new QStandardItemModel();
  if (!m_header_loaded)
  {
    ResultCode res = m_ftex.ReadHeader();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return res;
    }
  }
  m_ftex_header = m_ftex.GetHeader();
  int row = 0;

  // Magic
  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  CustomStandardItem* magic_item = new CustomStandardItem(m_ftex_header.magic);
  magic_item->SetFunction([this](QString text) { m_ftex_header.magic = text; });
  header_attributes_model->setItem(row, 1, magic_item);
  m_delegate_group.line_edit_delegates << 0;
  row++;

  // Header Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Header Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex.GetStart(), 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.data_offset, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Length"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.data_length, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Texture Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.mipmap_offset, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Size"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.mipmap_length, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Mipmaps"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.num_mips, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Width"));
  header_attributes_model->setItem(row, 1, new QStandardItem(QString::number(m_ftex_header.width)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Height"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem(QString::number(m_ftex_header.height)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Format"));
  QList<FTEX::FormatInfo> format_info_list = m_ftex.GetFormatInfoList();
  QStandardItemModel* format_combo_box_entries = new QStandardItemModel(format_info_list.size(), 0);
  for (int format = 0; format < format_info_list.size(); format++)
    format_combo_box_entries->setItem(format, new QStandardItem(format_info_list[format].name));
  m_delegate_group.combo_box_entries << format_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  m_delegate_group.combo_box_selections << m_ftex.GetFormatInfoIndex();
  header_attributes_model->setItem(row, 1, new QStandardItem(m_ftex.GetFormatInfo().name));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Tiling"));
  QList<FTEX::TileModeInfo> tile_mode_info_list = m_ftex.GetTileModeInfoList();
  QStandardItemModel* tiling_combo_box_entries =
      new QStandardItemModel(tile_mode_info_list.size(), 0);
  for (int tile_mode = 0; tile_mode < tile_mode_info_list.size(); tile_mode++)
    tiling_combo_box_entries->setItem(tile_mode, new QStandardItem(m_ftex.GetTileModeInfo().name));
  m_delegate_group.combo_box_entries << tiling_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  m_delegate_group.combo_box_selections << m_ftex_header.tile_mode;
  header_attributes_model->setItem(
      row, 1, new QStandardItem(tile_mode_info_list[m_ftex_header.tile_mode].name));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Usage"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.usage, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("AA Mode"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.aa_mode, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Swizzle"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.swizzle, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Depth"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.depth, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Dim"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.dim, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Pitch"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.pitch, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Alignment"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.alignment, 16)));
  row++;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  QObject::connect(header_attributes_model, SIGNAL(itemChanged(QStandardItem*)), this,
                   SLOT(HandleAttributeItemChange(QStandardItem*)));

  m_table_view = new QTableView;

  m_table_view->setModel(header_attributes_model);
  // stretch out table to fit space
  m_table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table_view->verticalHeader()->hide();
  m_table_view->horizontalHeader()->hide();

  m_table_view->setItemDelegate(new CustomItemDelegate(m_delegate_group));

  // To have all editors open by default, uncomment this out
  // PROS: Looks nicer, possibly more convienient
  // CONS: Scrolling can accidentally change values, and opening a new sections
  // seems to select the text
  // in every open editor for some reason
  // for (int i = 0; i < sectionHeaderModel->rowCount(); i++)
  // tableView->openPersistentEditor(sectionHeaderModel->index(i, 1));

  QVBoxLayout* attributes_layout = new QVBoxLayout();
  attributes_layout->addWidget(new QLabel("Header"));
  attributes_layout->addWidget(m_table_view);

  m_sections_container = new QScrollArea();
  m_sections_container->setLayout(attributes_layout);

  emit NewAttributesArea(m_sections_container);
  return ResultCode::Success;
}

ResultCode FTEXNode::LoadMainWidget()
{
  ResultCode res;
  if (!m_header_loaded)
    res = m_ftex.ReadHeader();
  if (!m_image_loaded && res != ResultCode::Success)
    res = m_ftex.ReadImage();
  if (res != ResultCode::Success)
  {
    emit NewStatus(res);
    return res;
  }
  QLabel* label = new QLabel("No in-editor texture viewing yet.");
  emit NewMainWidget(label);
  emit NewStatus(res);
  return ResultCode::Success;
}

void FTEXNode::HandleAttributeItemChange(QStandardItem* item)
{
#define DEBUG
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;
#ifdef DEBUG
  qDebug() << "Item changed. Row " << item->row() << " column " << item->column();
#endif
  CustomStandardItem* custom_item = dynamic_cast<CustomStandardItem*>(item);
  if (custom_item)
    custom_item->ExecuteFunction();

  m_ftex.SetHeader(m_ftex_header);
}

void FTEXNode::HandleExportActionClick()
{
  IODialog export_dialog(IODialog::Type::Export);
  QSettings settings;

  // GROUP: Output
  QGroupBox* output_group = new QGroupBox("Output");
  QVBoxLayout* output_layout = new QVBoxLayout();

  // Layout: File export path label, line edit, and button.
  QHBoxLayout* path_layout = new QHBoxLayout;

  path_layout->addWidget(new QLabel("Path: "));

  m_path_line_edit = new QLineEdit;
  settings.beginGroup("file_paths");
  QString last_path = settings.value("last_ftex_export_path").toString();
  if (!last_path.isEmpty())
  {
    QFileInfo file_info = last_path;
    m_path_line_edit->setText(file_info.dir().path() + QDir::separator() + m_ftex.GetName() + '.' +
                              file_info.suffix());
  }
  settings.endGroup();
  path_layout->addWidget(m_path_line_edit);

  QPushButton* path_button = new QPushButton("...");
  path_layout->addWidget(path_button);
  export_dialog.MakeSaveFilePathConnection(
      m_path_line_edit, path_button, "DirectDraw Surface Texture Format (*.dds);;All Files (*.*)");
  output_layout->addLayout(path_layout);

  // Layout: Format label and combo box.
  QHBoxLayout* format_layout = new QHBoxLayout;

  format_layout->addWidget(new QLabel("Format: "));

  m_format_combo_box = new QComboBox;
  m_format_combo_box->addItem("DDS");
  format_layout->addWidget(m_format_combo_box);
  output_layout->addLayout(format_layout);

  output_group->setLayout(output_layout);
  export_dialog.AddGroup(output_group);

  connect(&export_dialog, SIGNAL(StartIOAction()), this, SLOT(HandleExportButtonClick()));
  export_dialog.exec();
}

void FTEXNode::HandleExportButtonClick()
{
  QSettings settings;
  settings.beginGroup("file_paths");
  settings.setValue("last_ftex_export_path", m_path_line_edit->text());
  settings.endGroup();
  if (!m_header_loaded)
    m_ftex.ReadHeader();
  if (!m_image_loaded)
    m_ftex.ReadImage();
  if (m_format_combo_box->currentText() == "DDS")
    m_ftex.ExportToDDS(m_path_line_edit->text());
}

void FTEXNode::HandleInjectActionClick()
{
  IODialog inject_dialog(IODialog::Type::Inject);
  QSettings settings;

  QGroupBox* output_group = new QGroupBox("Output");

  QVBoxLayout* output_layout = new QVBoxLayout();

  // Layout: File export path label, line edit, and button.
  QHBoxLayout* path_layout = new QHBoxLayout;

  path_layout->addWidget(new QLabel("Path: "));

  m_path_line_edit = new QLineEdit;
  settings.beginGroup("file_paths");
  // Not a typo, the intended functionality is for it to use the path of the last exported DDS file.
  QString last_path = settings.value("last_ftex_export_path").toString();
  if (!last_path.isEmpty())
    m_path_line_edit->setText(last_path);
  settings.endGroup();
  path_layout->addWidget(m_path_line_edit);

  QPushButton* path_button = new QPushButton("...");
  path_layout->addWidget(path_button);
  inject_dialog.MakeOpenFilePathConnection(
      m_path_line_edit, path_button, "DirectDraw Surface Texture Format (*.dds);;All Files (*.*)");
  output_layout->addLayout(path_layout);

  // The problem with this if is that when adding the widget, it will take ownership of the sections
  // container and therefore delete it when the window is closed despite it possible still being
  // needed.
  // if (!m_sections_container)
  LoadAttributeArea();
  output_layout->addWidget(m_sections_container);

  output_group->setLayout(output_layout);

  inject_dialog.AddGroup(output_group);

  connect(&inject_dialog, SIGNAL(StartIOAction()), this, SLOT(HandleInjectButtonClick()));
  inject_dialog.exec();
}

void FTEXNode::HandleInjectButtonClick()
{
  QSettings settings;
  m_ftex.ImportDDS(m_path_line_edit->text());
  m_ftex.InjectImage();
}
