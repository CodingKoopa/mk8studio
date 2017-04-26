#include "FTEXNode.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

#include "ExportDialog.h"

QStandardItem* FTEXNode::MakeItem()
{
  QStandardItem* item = new QStandardItem();
  item->setData(QString(m_ftex->GetName() + " (FTEX)"), Qt::DisplayRole);
  item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  return item;
}

ResultCode FTEXNode::LoadAttributeArea()
{
  QStandardItemModel* header_attributes_model = new QStandardItemModel();
  // TODO: get result code from this
  if (!m_header_loaded)
    m_ftex->ReadHeader();
  m_header = m_ftex->GetHeader();
  int row = 0;

  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  header_attributes_model->setItem(row, 1, new QStandardItem(m_header.magic));
  m_delegate_group.line_edit_delegates << 0;
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Header Offset"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem(QString::number(m_ftex->GetStart(), 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_header.data_offset, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Length"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_header.data_length, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Texture Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_header.mipmap_offset, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Size"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_header.mipmap_length, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Mipmaps"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_header.num_mips, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Width"));
  header_attributes_model->setItem(row, 1, new QStandardItem(QString::number(m_header.width)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Height"));
  header_attributes_model->setItem(row, 1, new QStandardItem(QString::number(m_header.height)));
  row++;

  // TODO: make this a combobox with all available values
  header_attributes_model->setItem(row, 0, new QStandardItem("Format"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_header.format, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Usage"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_header.usage, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Tiling"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_header.tile_mode, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("AA Mode"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_header.aa_mode, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Swizzle"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_header.swizzle, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Depth"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_header.depth, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Dim"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_header.dim, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Pitch"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_header.pitch, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Alignment"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_header.alignment, 16)));
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

  m_table_view->setItemDelegate(new CustomDelegate(m_delegate_group));

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

  QScrollArea* sectionsContainer = new QScrollArea();
  sectionsContainer->setLayout(attributes_layout);

  emit NewAttributesArea(sectionsContainer);
  return RESULT_SUCCESS;
}

ResultCode FTEXNode::LoadMainWidget()
{
  ResultCode res;
  if (!m_header_loaded)
    m_ftex->ReadHeader();
  if (!m_image_loaded)
    res = m_ftex->ReadImageData();
  if (res != RESULT_SUCCESS)
  {
    emit NewStatus(res);
    return res;
  }
  QLabel* label = new QLabel("No in-editor texture viewing yet.");
  emit NewMainWidget(label);
  emit NewStatus(res);
  return RESULT_SUCCESS;
}

void FTEXNode::HandleAttributeItemChange(QStandardItem* item)
{
  // this is where changes made in the table would be handled
  // see: BFRESGUI::handleHeaderItemChange
  // (temp obviously)
  item = item;
}

void FTEXNode::HandleExportActionClick()
{
  QSettings settings;

  ExportDialog export_dialog;

  QGroupBox* output_group = new QGroupBox("Output");
  m_groups_list.append(output_group);
  QVBoxLayout* output_layout = new QVBoxLayout();

  // Path to the exported file.
  QHBoxLayout* path_layout = new QHBoxLayout;
  QLabel* path_text = new QLabel("Path: ");
  path_layout->addWidget(path_text);
  m_path_line_edit = new QLineEdit;
  settings.beginGroup("file_paths");
  if (!settings.value("last_ftex_export_path").toString().isEmpty())
    m_path_line_edit->setText(settings.value("last_ftex_export_path").toString());
  settings.endGroup();
  path_layout->addWidget(m_path_line_edit);
  QPushButton* path_button = new QPushButton("...");
  path_layout->addWidget(path_button);
  export_dialog.ConnectFilePathPair(m_path_line_edit, path_button);
  output_layout->addLayout(path_layout);

  // What format to export to.
  QHBoxLayout* format_layout = new QHBoxLayout;
  QLabel* format_text = new QLabel("Format: ");
  format_layout->addWidget(format_text);
  QComboBox* format_combo_box = new QComboBox;
  format_combo_box->addItem("DDS");
  format_layout->addWidget(format_combo_box);
  output_layout->addLayout(format_layout);

  output_group->setLayout(output_layout);
  export_dialog.AddGroup(output_group);

  connect(&export_dialog, SIGNAL(StartExport()), this, SLOT(HandleExportButtonClick()));
  export_dialog.exec();
}

void FTEXNode::HandleExportButtonClick()
{
  QSettings settings;
  settings.beginGroup("file_paths");
  settings.setValue("last_ftex_export_path", m_path_line_edit->text());
  settings.endGroup();
  m_ftex->SetName(m_path_line_edit->text());
  if (!m_header_loaded)
    m_ftex->ReadHeader();
  if (!m_image_loaded)
    m_ftex->ReadImageData();
  m_ftex->ExportToDDS();
}
