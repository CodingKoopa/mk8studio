#include "Nodes/Textures/FTEXNode.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSettings>

#include "Common.h"
#include "IODialog.h"

FTEXNode::FTEXNode(std::shared_ptr<FTEX> ftex, QObject* parent) : Node(parent), m_ftex(ftex)
{
  m_context_menu = new QMenu;

  QAction* action_export = new QAction("Export", this);
  connect(action_export, &QAction::triggered, this, &FTEXNode::HandleExportAction);
  m_context_menu->addAction(action_export);

  QAction* action_inject = new QAction("Inject", this);
  connect(action_inject, &QAction::triggered, this, &FTEXNode::HandleInjectAction);
  m_context_menu->addAction(action_inject);
}

DynamicStandardItem* FTEXNode::MakeItem()
{
  DynamicStandardItem* item = new DynamicStandardItem;
  item->setData(m_ftex->GetName() + " (FTEX)", Qt::DisplayRole);
  item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  emit ConnectNode(this);
  return item;
}

ResultCode FTEXNode::LoadAttributeArea()
{
  if (!m_header_loaded)
  {
    ResultCode res = m_ftex->ReadHeader();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return res;
    }
    else
      m_ftex_header = m_ftex->GetHeader();
  }

  quint32 row = 0;
  QStandardItemModel* header_attributes_model = new QStandardItemModel;
  //  m_delegate_group = DynamicItemDelegate::DelegateInfo();

  // Magic
  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  DynamicStandardItem* magic_item = new DynamicStandardItem(m_ftex_header.magic);
  magic_item->SetFunction([this](QString text) { m_ftex_header.magic = text; });
  header_attributes_model->setItem(row, 1, magic_item);
  //  m_delegate_group.line_edit_delegates << row;
  ++row;

  // Dimension
  // (TODO: Enum combo box for this.)
  header_attributes_model->setItem(row, 0, new QStandardItem("Dimension"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.dimension, 16)));
  ++row;

  // Texture Width
  header_attributes_model->setItem(row, 0, new QStandardItem("Width"));
  DynamicStandardItem* width_item = new DynamicStandardItem(QString::number(m_ftex_header.width));
  width_item->SetFunction([this](QString text) { m_ftex_header.width = text.toUInt(); });
  header_attributes_model->setItem(row, 1, width_item);
  //  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Texture Height
  header_attributes_model->setItem(row, 0, new QStandardItem("Height"));
  DynamicStandardItem* height_item = new DynamicStandardItem(QString::number(m_ftex_header.height));
  height_item->SetFunction([this](QString text) { m_ftex_header.height = text.toUInt(); });
  header_attributes_model->setItem(row, 1, height_item);
  //  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Texture Depth
  header_attributes_model->setItem(row, 0, new QStandardItem("Depth"));
  DynamicStandardItem* depth_item = new DynamicStandardItem(QString::number(m_ftex_header.depth));
  depth_item->SetFunction([this](QString text) { m_ftex_header.depth = text.toUInt(); });
  header_attributes_model->setItem(row, 1, depth_item);
  ++row;

  // Number Of Mipmaps
  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Mipmaps"));
  DynamicStandardItem* num_mipmaps_item =
      new DynamicStandardItem(QString::number(m_ftex_header.num_mips));
  num_mipmaps_item->SetFunction([this](QString text) { m_ftex_header.num_mips = text.toUInt(); });
  header_attributes_model->setItem(row, 1, num_mipmaps_item);
  ++row;

  // Texture Format
  header_attributes_model->setItem(row, 0, new QStandardItem("Format"));
  auto format_infos = m_ftex->GetFormatInfos();
  QStandardItemModel* format_combo_box_entries = new QStandardItemModel();
  for (auto const& format_info : format_infos)
    format_combo_box_entries->appendRow(new QStandardItem(format_info.second.name));
  //  m_delegate_group.combo_box_entries << format_combo_box_entries;
  //  m_delegate_group.combo_box_delegates << row;
  //  m_delegate_group.combo_box_selections
  //  << std::distance(format_infos.begin(), format_infos.find(m_ftex_header.format));
  DynamicStandardItem* format_item = new DynamicStandardItem(m_ftex->GetFormatInfo().name);
  // Here, index is the index of the overall map, and NOT a key.
  format_item->SetFunction([this, format_infos](quint32 index) {
    auto it =
        next(format_infos.begin(), std::min(index, static_cast<quint32>(format_infos.size())));
    m_ftex_header.format = it->first;
  });
  header_attributes_model->setItem(row, 1, format_item);
  ++row;

  // Anti-Aliasing
  // (TODO: Enum combo box for this.)
  header_attributes_model->setItem(row, 0, new QStandardItem("AA Mode"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.aa_mode, 16)));
  ++row;

  // Usage
  // (TODO: Enum combo box for this.)
  header_attributes_model->setItem(row, 0, new QStandardItem("Usage"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.usage, 16)));
  ++row;

  // Texture Size
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Size"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.data_length, 16)));
  ++row;

  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Runtime Pointer"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.data_offset_runtime, 16)));
  ++row;

  // Mipmap Size
  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Size"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.mipmap_length, 16)));
  ++row;

  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Runtime Pointer"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.mipmap_offset_runtime, 16)));
  ++row;

  // Tile Mode
  header_attributes_model->setItem(row, 0, new QStandardItem("Tile Mode"));
  auto tile_mode_infos = m_ftex->GetTileModeInfos();
  QStandardItemModel* tiling_combo_box_entries = new QStandardItemModel();
  for (auto const& tile_mode_info : tile_mode_infos)
    tiling_combo_box_entries->appendRow(new QStandardItem(tile_mode_info.second.name));
  //  m_delegate_group.combo_box_entries << tiling_combo_box_entries;
  //  m_delegate_group.combo_box_delegates << row;
  // The tile mode corresponds with the index because they're consecutive.
  //  m_delegate_group.combo_box_selections << m_ftex_header.tile_mode;
  DynamicStandardItem* tile_mode_item = new DynamicStandardItem(m_ftex->GetTileModeInfo().name);
  tile_mode_item->SetFunction(
      [this](quint32 index) { m_ftex_header.tile_mode = static_cast<quint32>(index); });
  header_attributes_model->setItem(row, 1, tile_mode_item);
  ++row;

  // Swizzle
  header_attributes_model->setItem(row, 0, new QStandardItem("Swizzle"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.swizzle, 16)));
  ++row;

  // Alignment
  header_attributes_model->setItem(row, 0, new QStandardItem("Alignment"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.alignment, 16)));
  ++row;

  // Pitch
  header_attributes_model->setItem(row, 0, new QStandardItem("Pitch"));
  DynamicStandardItem* pitch_item = new DynamicStandardItem(QString::number(m_ftex_header.pitch));
  pitch_item->SetFunction([this](QString text) { m_ftex_header.pitch = text.toUInt(); });
  header_attributes_model->setItem(row, 1, pitch_item);
  //  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Mipmap Offsets
  for (quint32 mipmap = 0; mipmap < m_ftex_header.num_mips; ++mipmap)
  {
    header_attributes_model->setItem(row, 0,
                                     new QStandardItem("Mipmap " + QString::number(mipmap)));
    header_attributes_model->setItem(
        row, 1,
        new QStandardItem("0x" + QString::number(m_ftex_header.mipmap_offsets[mipmap], 16)));
    ++row;
  }

  // First Mipmap
  header_attributes_model->setItem(row, 0, new QStandardItem("First Mipmap"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem(QString::number(m_ftex_header.first_mipmap)));
  ++row;

  // Number Of Mipmaps Again
  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Mipmaps"));
  DynamicStandardItem* num_mipmaps_alt_item =
      new DynamicStandardItem(QString::number(m_ftex_header.num_mips_alt));
  num_mipmaps_alt_item->SetFunction(
      [this](QString text) { m_ftex_header.num_mips_alt = text.toUInt(); });
  header_attributes_model->setItem(row, 1, num_mipmaps_alt_item);
  //  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Number of Slices
  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Slices"));
  DynamicStandardItem* num_slices_item =
      new DynamicStandardItem(QString::number(m_ftex_header.num_slices));
  num_slices_item->SetFunction([this](QString text) { m_ftex_header.num_slices = text.toUInt(); });
  header_attributes_model->setItem(row, 1, num_slices_item);
  //  m_delegate_group.spin_box_delegates << row;
  ++row;

  auto component_names = m_ftex->GetComponentNames();
  QStandardItemModel* component_selector_combo_box_entries = new QStandardItemModel();
  for (quint32 component = 0; component < component_names.size(); ++component)
    component_selector_combo_box_entries->appendRow(new QStandardItem(component_names[component]));

  // Red Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Red Channel Binding"));
  //  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  //  m_delegate_group.combo_box_delegates << row;
  //  m_delegate_group.combo_box_selections << m_ftex_header.red_channel_component;
  DynamicStandardItem* red_channel_component_item =
      new DynamicStandardItem(component_names[m_ftex_header.red_channel_component]);
  red_channel_component_item->SetFunction(
      [this, component_names](quint32 index) { m_ftex_header.red_channel_component = index; });
  header_attributes_model->setItem(row, 1, red_channel_component_item);
  ++row;

  // Green Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Green Channel Binding"));
  //  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  //  m_delegate_group.combo_box_delegates << row;
  //  m_delegate_group.combo_box_selections << m_ftex_header.green_channel_component;
  DynamicStandardItem* green_channel_component_item =
      new DynamicStandardItem(component_names[m_ftex_header.green_channel_component]);
  green_channel_component_item->SetFunction(
      [this, component_names](quint32 index) { m_ftex_header.green_channel_component = index; });
  header_attributes_model->setItem(row, 1, green_channel_component_item);
  ++row;

  // Blue Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Blue Channel Binding"));
  //  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  //  m_delegate_group.combo_box_delegates << row;
  //  m_delegate_group.combo_box_selections << m_ftex_header.blue_channel_component;
  DynamicStandardItem* blue_channel_component_item =
      new DynamicStandardItem(component_names[m_ftex_header.blue_channel_component]);
  blue_channel_component_item->SetFunction(
      [this, component_names](quint32 index) { m_ftex_header.blue_channel_component = index; });
  header_attributes_model->setItem(row, 1, blue_channel_component_item);
  ++row;

  // Red Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Alpha Channel Binding"));
  //  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  //  m_delegate_group.combo_box_delegates << row;
  //  m_delegate_group.combo_box_selections << m_ftex_header.alpha_channel_component;
  DynamicStandardItem* alpha_channel_component_item =
      new DynamicStandardItem(component_names[m_ftex_header.alpha_channel_component]);
  alpha_channel_component_item->SetFunction(
      [this, component_names](quint32 index) { m_ftex_header.alpha_channel_component = index; });
  header_attributes_model->setItem(row, 1, alpha_channel_component_item);
  ++row;

  // Texture Registers
  for (qint32 texture_register = 0; texture_register < m_ftex_header.registers.size();
       ++texture_register)
  {
    header_attributes_model->setItem(
        row, 0, new QStandardItem("Register " + QString::number(texture_register)));
    header_attributes_model->setItem(
        row, 1,
        new QStandardItem("0x" + QString::number(m_ftex_header.registers[texture_register], 16)));
    ++row;
  }

  // Texture Runtime Handle
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Runtime Handle"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.texture_handle_runtime, 16)));
  ++row;

  // Array Length
  header_attributes_model->setItem(row, 0, new QStandardItem("Array Length"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.array_length, 16)));
  ++row;

  // File Name Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("File Name Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.file_name_offset, 16)));
  ++row;

  // File Path Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("File Path Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.file_path_offset, 16)));
  ++row;

  // Texture Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Data Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.data_offset, 16)));
  ++row;

  // Mipmap Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Data Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.file_path_offset, 16)));
  ++row;

  // User Data Index Group Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("User Data Index Group Offset"));
  header_attributes_model->setItem(
      row, 1,
      new QStandardItem("0x" + QString::number(m_ftex_header.user_data_index_group_offset, 16)));
  ++row;

  // User Data Entry Count
  header_attributes_model->setItem(row, 0, new QStandardItem("User Data Entry Count"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.user_data_entry_count, 16)));
  ++row;

  // Texture Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.data_offset, 16)));
  ++row;

  // Mipmap Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.mipmap_section_offset, 16)));
  ++row;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  connect(header_attributes_model, &QStandardItemModel::itemChanged, this,
          &FTEXNode::HandleAttributeItemChange);

  m_sections_container = MakeAttributeSectionOld(header_attributes_model);

  emit NewAttributeArea(m_sections_container);
  return ResultCode::Success;
}

ResultCode FTEXNode::LoadMainWidget()
{
  ResultCode res;
  if (!m_header_loaded)
    res = m_ftex->ReadHeader();
  if (!m_image_loaded && res != ResultCode::Success)
    res = m_ftex->ReadImage();
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
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;
  DynamicStandardItem* custom_item = dynamic_cast<DynamicStandardItem*>(item);
  if (custom_item)
    custom_item->ExecuteFunction();

  m_ftex->SetHeader(m_ftex_header);
}

void FTEXNode::HandleExportAction()
{
  IODialog export_dialog(IODialog::Type::Export, m_last_export_path_key, m_ftex->GetName(),
                         "DirectDraw Surface Texture Format (*.dds);;All Files (*.*)",
                         QStandardPaths::PicturesLocation, "dds");

  QLabel* format_label = new QLabel("Format: ");

  m_format_combo_box = new QComboBox;
  m_format_combo_box->addItem(m_dds_item_string);

  QHBoxLayout* format_layout = new QHBoxLayout;
  format_layout->addWidget(format_label);
  format_layout->addWidget(m_format_combo_box);

  QGroupBox* output_group = new QGroupBox;
  output_group->setLayout(format_layout);
  export_dialog.AddGroup(output_group);

  connect(&export_dialog, &IODialog::StartAction, this, &FTEXNode::HandleExportButton);
  export_dialog.exec();
}

void FTEXNode::HandleExportButton(const QString& path)
{
  QSettings settings;
  settings.beginGroup("file_paths");
  settings.setValue(m_last_export_path_key, path);
  settings.endGroup();
  if (!m_header_loaded)
    m_ftex->ReadHeader();
  if (!m_image_loaded)
    m_ftex->ReadImage();
  if (m_format_combo_box->currentText() == "DDS")
    m_ftex->ExportToDDS(path);
}

void FTEXNode::HandleInjectAction()
{
  IODialog inject_dialog(IODialog::Type::Inject, m_last_inject_path_key, m_ftex->GetName(),
                         "DirectDraw Surface Texture Format (*.dds);;All Files (*.*)",
                         QStandardPaths::PicturesLocation, "dds");
  connect(&inject_dialog, &IODialog::StartAction, this, &FTEXNode::HandleInjectButton);
  inject_dialog.exec();
}

void FTEXNode::HandleInjectButton(const QString& path)
{
  QSettings settings;
  settings.beginGroup("file_paths");
  settings.setValue(m_last_inject_path_key, path);
  settings.endGroup();
  m_ftex->ImportDDS(path);
  m_ftex->InjectImage();
}
