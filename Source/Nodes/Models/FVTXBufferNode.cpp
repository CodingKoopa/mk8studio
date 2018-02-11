#include "Nodes/Models/FVTXBufferNode.h"
#include "CustomStandardItem.h"
#include "Formats/Models/FVTX.h"

FVTXBufferNode::FVTXBufferNode(const FVTX::Buffer& buffer, QObject* parent)
    : Node(parent), m_buffer(buffer)
{
}

CustomStandardItem* FVTXBufferNode::MakeItem()
{
  return MakeLabelItem("Buffer");
}

ResultCode FVTXBufferNode::LoadAttributeArea()
{
  quint32 row = 0;
  QStandardItemModel* header_attributes_model = new QStandardItemModel;
  m_delegate_group = CustomItemDelegate::DelegateGroup();

  // Data Pointer (Runtime)
  header_attributes_model->setItem(row, 0, new QStandardItem("Runtime Data Pointer"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_buffer.data_pointer_runtime, 16)));
  ++row;

  // Size
  header_attributes_model->setItem(row, 0, new QStandardItem("Size"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_buffer.size, 16)));
  ++row;

  // Runtime Buffer Handle
  header_attributes_model->setItem(row, 0, new QStandardItem("Runtime Buffer Handle"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_buffer.buffer_handle_runtime, 16)));
  ++row;

  // Stride
  header_attributes_model->setItem(row, 0, new QStandardItem("Stride"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(m_buffer.stride, 16)));
  ++row;

  // Buffering Count
  header_attributes_model->setItem(row, 0, new QStandardItem("Buffering Count"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_buffer.buffering_count, 16)));
  ++row;

  // Runtime Context Pointer
  header_attributes_model->setItem(row, 0, new QStandardItem("Runtime Context Pointer"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_buffer.context_pointer_runtime, 16)));
  ++row;

  // Data Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("Data Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_buffer.data_offset, 16)));
  ++row;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  connect(header_attributes_model, &QStandardItemModel::itemChanged, this,
          &FVTXBufferNode::HandleAttributeItemChange);

  emit NewAttributesArea(MakeAttributeSection(header_attributes_model));
  return ResultCode::Success;
}

void FVTXBufferNode::HandleAttributeItemChange(QStandardItem* item)
{
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;

  CustomStandardItem* custom_item = dynamic_cast<CustomStandardItem*>(item);
  if (custom_item)
    custom_item->ExecuteFunction();

  // TODO: See #7.
}
