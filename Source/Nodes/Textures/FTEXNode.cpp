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

FTEXNode::FTEXNode(const FTEX& ftex, QObject* parent) : Node(parent), m_ftex(ftex)
{
  m_context_menu = new QMenu;

  QAction* action_export = new QAction("Export", this);
  connect(action_export, &QAction::triggered, this, &FTEXNode::HandleExportActionClick);
  m_context_menu->addAction(action_export);

  QAction* action_inject = new QAction("Inject", this);
  connect(action_inject, &QAction::triggered, this, &FTEXNode::HandleInjectActionClick);
  m_context_menu->addAction(action_inject);
}

CustomStandardItem* FTEXNode::MakeItem()
{
  CustomStandardItem* item = new CustomStandardItem;
  item->setData(m_ftex.GetName() + " (FTEX)", Qt::DisplayRole);
  item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  return item;
}

ResultCode FTEXNode::LoadAttributeArea()
{
  if (!m_header_loaded)
  {
    ResultCode res = m_ftex.ReadHeader();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return res;
    }
    else
      m_ftex_header = m_ftex.GetHeader();
  }

  quint32 row = 0;
  QStandardItemModel* header_attributes_model = new QStandardItemModel;
  m_delegate_group = CustomItemDelegate::DelegateGroup();

  // TODO: Hex Spinbox

  // Header Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Header Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex.GetStart(), 16)));
  ++row;

  // Magic
  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  CustomStandardItem* magic_item = new CustomStandardItem(m_ftex_header.magic);
  magic_item->SetFunction([this](QString text) { m_ftex_header.magic = text; });
  header_attributes_model->setItem(row, 1, magic_item);
  m_delegate_group.line_edit_delegates << row;
  ++row;

  // Dimension
  // (TODO: Enum combo box for this.)
  header_attributes_model->setItem(row, 0, new QStandardItem("Dimension"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_ftex_header.dimension, 16)));
  ++row;

  // Texture Width
  header_attributes_model->setItem(row, 0, new QStandardItem("Width"));
  CustomStandardItem* width_item = new CustomStandardItem(QString::number(m_ftex_header.width));
  width_item->SetFunction([this](QString text) { m_ftex_header.width = text.toUInt(); });
  header_attributes_model->setItem(row, 1, width_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Texture Height
  header_attributes_model->setItem(row, 0, new QStandardItem("Height"));
  CustomStandardItem* height_item = new CustomStandardItem(QString::number(m_ftex_header.height));
  height_item->SetFunction([this](QString text) { m_ftex_header.height = text.toUInt(); });
  header_attributes_model->setItem(row, 1, height_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Texture Depth
  header_attributes_model->setItem(row, 0, new QStandardItem("Depth"));
  CustomStandardItem* depth_item = new CustomStandardItem(QString::number(m_ftex_header.depth));
  depth_item->SetFunction([this](QString text) { m_ftex_header.depth = text.toUInt(); });
  header_attributes_model->setItem(row, 1, depth_item);
  ++row;

  // Number Of Mipmaps
  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Mipmaps"));
  CustomStandardItem* num_mipmaps_item =
      new CustomStandardItem(QString::number(m_ftex_header.num_mips));
  num_mipmaps_item->SetFunction([this](QString text) { m_ftex_header.num_mips = text.toUInt(); });
  header_attributes_model->setItem(row, 1, num_mipmaps_item);
  ++row;

  // Texture Format
  header_attributes_model->setItem(row, 0, new QStandardItem("Format"));
  auto format_infos = m_ftex.GetFormatInfos();
  QStandardItemModel* format_combo_box_entries = new QStandardItemModel();
  for (auto const& format_info : format_infos)
    format_combo_box_entries->appendRow(new QStandardItem(format_info.second.name));
  m_delegate_group.combo_box_entries << format_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  m_delegate_group.combo_box_selections
      << std::distance(format_infos.begin(), format_infos.find(m_ftex_header.format));
  CustomStandardItem* format_item = new CustomStandardItem(m_ftex.GetFormatInfo().name);
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
  auto tile_mode_infos = m_ftex.GetTileModeInfos();
  QStandardItemModel* tiling_combo_box_entries = new QStandardItemModel();
  for (auto const& tile_mode_info : tile_mode_infos)
    tiling_combo_box_entries->appendRow(new QStandardItem(tile_mode_info.second.name));
  m_delegate_group.combo_box_entries << tiling_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  // The tile mode corresponds with the index because they're consecutive.
  m_delegate_group.combo_box_selections << m_ftex_header.tile_mode;
  CustomStandardItem* tile_mode_item = new CustomStandardItem(m_ftex.GetTileModeInfo().name);
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
  CustomStandardItem* pitch_item = new CustomStandardItem(QString::number(m_ftex_header.pitch));
  pitch_item->SetFunction([this](QString text) { m_ftex_header.pitch = text.toUInt(); });
  header_attributes_model->setItem(row, 1, pitch_item);
  m_delegate_group.spin_box_delegates << row;
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
  CustomStandardItem* num_mipmaps_alt_item =
      new CustomStandardItem(QString::number(m_ftex_header.num_mips_alt));
  num_mipmaps_alt_item->SetFunction(
      [this](QString text) { m_ftex_header.num_mips_alt = text.toUInt(); });
  header_attributes_model->setItem(row, 1, num_mipmaps_alt_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Number of Slices
  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Slices"));
  CustomStandardItem* num_slices_item =
      new CustomStandardItem(QString::number(m_ftex_header.num_slices));
  num_slices_item->SetFunction([this](QString text) { m_ftex_header.num_slices = text.toUInt(); });
  header_attributes_model->setItem(row, 1, num_slices_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  QVector<QString> component_name_list = m_ftex.GetComponentNameList();
  QStandardItemModel* component_selector_combo_box_entries = new QStandardItemModel();
  component_selector_combo_box_entries->appendRow(new QStandardItem(component_name_list[0]));
  component_selector_combo_box_entries->appendRow(new QStandardItem(component_name_list[1]));
  component_selector_combo_box_entries->appendRow(new QStandardItem(component_name_list[2]));
  component_selector_combo_box_entries->appendRow(new QStandardItem(component_name_list[3]));
  component_selector_combo_box_entries->appendRow(new QStandardItem(component_name_list[4]));
  component_selector_combo_box_entries->appendRow(new QStandardItem(component_name_list[5]));

  // Red Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Red Channel Binding"));
  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  m_delegate_group.combo_box_selections << m_ftex_header.red_channel_component;
  CustomStandardItem* red_channel_component_item =
      new CustomStandardItem(component_name_list[m_ftex_header.red_channel_component]);
  red_channel_component_item->SetFunction([this](QString text) {
    m_ftex_header.red_channel_component = m_ftex.GetComponentIDFromName(text);
  });
  header_attributes_model->setItem(row, 1, red_channel_component_item);
  ++row;

  // Green Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Green Channel Binding"));
  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  m_delegate_group.combo_box_selections << m_ftex_header.green_channel_component;
  CustomStandardItem* green_channel_component_item =
      new CustomStandardItem(component_name_list[m_ftex_header.green_channel_component]);
  green_channel_component_item->SetFunction([this](QString text) {
    m_ftex_header.green_channel_component = m_ftex.GetComponentIDFromName(text);
  });
  header_attributes_model->setItem(row, 1, green_channel_component_item);
  ++row;

  // Blue Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Blue Channel Binding"));
  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  m_delegate_group.combo_box_selections << m_ftex_header.blue_channel_component;
  CustomStandardItem* blue_channel_component_item =
      new CustomStandardItem(component_name_list[m_ftex_header.blue_channel_component]);
  blue_channel_component_item->SetFunction([this](QString text) {
    m_ftex_header.blue_channel_component = m_ftex.GetComponentIDFromName(text);
  });
  header_attributes_model->setItem(row, 1, blue_channel_component_item);
  ++row;

  // Alpha Channel Selector
  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Alpha Channel Binding"));
  m_delegate_group.combo_box_entries << component_selector_combo_box_entries;
  m_delegate_group.combo_box_delegates << row;
  m_delegate_group.combo_box_selections << m_ftex_header.alpha_channel_component;
  CustomStandardItem* alpha_channel_component_item =
      new CustomStandardItem(component_name_list[m_ftex_header.alpha_channel_component]);
  alpha_channel_component_item->SetFunction([this](QString text) {
    m_ftex_header.alpha_channel_component = m_ftex.GetComponentIDFromName(text);
  });
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

  m_sections_container = MakeAttributeSection(header_attributes_model);

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
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;
  CustomStandardItem* custom_item = dynamic_cast<CustomStandardItem*>(item);
  if (custom_item)
    custom_item->ExecuteFunction();

  m_ftex.SetHeader(m_ftex_header);
  m_ftex.SetupInfo();
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

  connect(&export_dialog, &IODialog::StartIOAction, this, &FTEXNode::HandleExportButtonClick);
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

  connect(&inject_dialog, &IODialog::StartIOAction, this, &FTEXNode::HandleInjectButtonClick);
  inject_dialog.exec();
}

void FTEXNode::HandleInjectButtonClick()
{
  QSettings settings;
  m_ftex.ImportDDS(m_path_line_edit->text());
  m_ftex.InjectImage();
}
