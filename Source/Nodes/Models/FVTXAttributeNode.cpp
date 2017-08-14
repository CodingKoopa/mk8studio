#include "FVTXAttributeNode.h"
#include "CustomStandardItem.h"

FVTXAttributeNode::FVTXAttributeNode(const FVTX::Attribute& attribute, QObject* parent)
    : Node(parent), m_attribute(attribute)
{
}

CustomStandardItem* FVTXAttributeNode::MakeItem()
{
  // The header has, by design, already been read by the FVTX class.
  CustomStandardItem* attribute_item = new CustomStandardItem;
  attribute_item->setData(m_attribute.name_info.friendly_name, Qt::DisplayRole);
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

  // Friendly Name
  header_attributes_model->setItem(row, 0, new QStandardItem("Friendly Name"));
  header_attributes_model->setItem(row, 1, new QStandardItem(m_attribute.name_info.friendly_name));
  ++row;

  // Internal Name
  header_attributes_model->setItem(row, 0, new QStandardItem("Internal Name"));
  CustomStandardItem* magic_item = new CustomStandardItem(m_attribute.name_info.internal_name);
  magic_item->SetFunction([this](QString text) { m_attribute.name_info.internal_name = text; });
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
