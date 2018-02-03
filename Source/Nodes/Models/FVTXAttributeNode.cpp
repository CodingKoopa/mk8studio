#include "FVTXAttributeNode.h"
#include "CustomStandardItem.h"

FVTXAttributeNode::FVTXAttributeNode(const FVTX::Attribute& attribute,
                                     QString attribute_friendly_name, QObject* parent)
    : Node(parent), m_attribute(attribute), m_attribute_friendly_name(attribute_friendly_name)
{
}

CustomStandardItem* FVTXAttributeNode::MakeItem()
{
  // The header has, by design, already been read by the FVTX class.
  CustomStandardItem* attribute_item = new CustomStandardItem;
  attribute_item->setData(m_attribute_friendly_name, Qt::DisplayRole);
  attribute_item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  return attribute_item;
}

ResultCode FVTXAttributeNode::LoadAttributeArea()
{
  quint32 row = 0;
  QStandardItemModel* header_attributes_model = new QStandardItemModel;
  m_delegate_group = CustomItemDelegate::DelegateGroup();

  // Name Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Name Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_attribute.name_offset, 16)));
  ++row;

  // Buffer Index
  header_attributes_model->setItem(row, 0, new QStandardItem("Buffer Index"));
  CustomStandardItem* buffer_index_item =
      new CustomStandardItem(QString::number(m_attribute.buffer_index));
  // TODO: This is a char.
  buffer_index_item->SetFunction(
      [this](QString text) { m_attribute.buffer_index = text.toUShort(); });
  header_attributes_model->setItem(row, 1, buffer_index_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Buffer Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Buffer Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_attribute.buffer_offset, 16)));
  ++row;

  // TODO: Format
  // Texture Format
  //  header_attributes_model->setItem(row, 0, new QStandardItem("Format"));
  //  QList<FTEX::FormatInfo> format_info_list = m_ftex.GetFormatInfoList();
  //  QStandardItemModel* format_combo_box_entries = new QStandardItemModel(format_info_list.size(),
  //  0); for (int format = 0; format < format_info_list.size(); ++format)
  //    format_combo_box_entries->setItem(format, new QStandardItem(format_info_list[format].name));
  //  m_delegate_group.combo_box_entries << format_combo_box_entries;
  //  m_delegate_group.combo_box_delegates << row;
  //  m_delegate_group.combo_box_selections << m_ftex.GetFormatInfoIndex();
  //  CustomStandardItem* format_item = new CustomStandardItem(m_ftex.GetFormatInfo().name);
  //  format_item->SetFunction(
  //      [this](QString text) { m_ftex_header.format = m_ftex.GetFormatIDFromName(text); });
  //  header_attributes_model->setItem(row, 1, format_item);
  //  ++row;

  // Friendly Name
  header_attributes_model->setItem(row, 0, new QStandardItem("Friendly Name"));
  header_attributes_model->setItem(row, 1, new QStandardItem());
  ++row;

  // Internal Name
  header_attributes_model->setItem(row, 0, new QStandardItem("Internal Name"));
  CustomStandardItem* magic_item = new CustomStandardItem(m_attribute.name);
  magic_item->SetFunction([this](QString text) { m_attribute.name = text; });
  header_attributes_model->setItem(row, 1, magic_item);
  m_delegate_group.line_edit_delegates << row;
  ++row;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  connect(header_attributes_model, &QStandardItemModel::itemChanged, this,
          &FVTXAttributeNode::HandleAttributeItemChange);

  emit NewAttributesArea(MakeAttributeSection(header_attributes_model));
  return ResultCode::Success;
}

void FVTXAttributeNode::HandleAttributeItemChange(QStandardItem* item)
{
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;

  CustomStandardItem* custom_item = dynamic_cast<CustomStandardItem*>(item);
  if (custom_item)
    custom_item->ExecuteFunction();

  // m_attribute.SetHeader(m_fmdl_header);
}
