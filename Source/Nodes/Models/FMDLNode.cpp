#include "Nodes/Models/FMDLNode.h"
#include "Nodes/Models/FVTXNode.h"

FMDLNode::FMDLNode(const FMDL& fmdl, QObject* parent) : Node(parent), m_fmdl(fmdl) {}

CustomStandardItem* FMDLNode::MakeItem()
{
  if (!m_header_loaded)
  {
    ResultCode res = m_fmdl.ReadHeader();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return nullptr;
    }
    else
    {
      m_fmdl_header = m_fmdl.GetHeader();
      m_header_loaded = true;
    }
  }
  if (!m_fvtxs_loaded)
  {
    ResultCode res = m_fmdl.ReadFVTXArray();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return nullptr;
    }
    else
    {
      m_fvtx_list = m_fmdl.GetFVTXList();
      m_fvtxs_loaded = true;
    }
  }

  CustomStandardItem* fmdl_item = new CustomStandardItem;
  fmdl_item->setData(m_fmdl.GetName() + " (FMDL)", Qt::DisplayRole);
  fmdl_item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  CustomStandardItem* fvtx_group_item = new CustomStandardItem("FVTX Sections");
  fmdl_item->appendRow(fvtx_group_item);
  foreach (const FVTX& fvtx, m_fvtx_list)
  {
    FVTXNode* fvtx_node = new FVTXNode(fvtx, this);
    connect(fvtx_node, &FVTXNode::ConnectNode, this, &FMDLNode::ConnectNode);
    emit ConnectNode(fvtx_node);
    fvtx_group_item->appendRow(fvtx_node->MakeItem());
  }
  return fmdl_item;
}

ResultCode FMDLNode::LoadAttributeArea()
{
  ResultCode res;
  if (!m_header_loaded)
  {
    res = m_fmdl.ReadHeader();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return res;
    }
    else
    {
      m_fmdl_header = m_fmdl.GetHeader();
      m_header_loaded = true;
    }
  }

  quint32 row = 0;
  QStandardItemModel* header_attributes_model = new QStandardItemModel;
  m_delegate_group = CustomItemDelegate::DelegateGroup();

  // Magic
  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  CustomStandardItem* magic_item = new CustomStandardItem(m_fmdl_header.magic);
  magic_item->SetFunction([this](QString text) { m_fmdl_header.magic = text; });
  header_attributes_model->setItem(row, 1, magic_item);
  m_delegate_group.line_edit_delegates << row;
  ++row;

  // File Name Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("File Name Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_fmdl_header.file_name_offset, 16)));
  ++row;

  // File Path Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("File Path Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_fmdl_header.file_path_offset, 16)));
  ++row;

  // FSKL Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("FSKL Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_fmdl_header.fskl_offset, 16)));
  ++row;

  // FVTX Array Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("FVTX Array Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_fmdl_header.fvtx_array_offset, 16)));
  ++row;

  // FSHP Index Group Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("FSHP Index Group Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_fmdl_header.fshp_index_group_offset, 16)));
  ++row;

  // FMAT Index Group Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("FMAT Index Group Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_fmdl_header.fmat_index_group_offset, 16)));
  ++row;

  // User Data Index Group Offset
  header_attributes_model->setItem(row, 0, new QStandardItem("User Data Index Group Offset"));
  header_attributes_model->setItem(
      row, 1,
      new QStandardItem("0x" + QString::number(m_fmdl_header.user_data_index_group_offset, 16)));
  ++row;

  // FVTX Count
  header_attributes_model->setItem(row, 0, new QStandardItem("FVTX Count"));
  CustomStandardItem* fvtx_count_item =
      new CustomStandardItem(QString::number(m_fmdl_header.fvtx_count));
  fvtx_count_item->SetFunction(
      [this](QString text) { m_fmdl_header.fvtx_count = text.toUShort(); });
  header_attributes_model->setItem(row, 1, fvtx_count_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // FSHP Count
  header_attributes_model->setItem(row, 0, new QStandardItem("FSHP Count"));
  CustomStandardItem* fshp_count_item =
      new CustomStandardItem(QString::number(m_fmdl_header.fshp_count));
  fshp_count_item->SetFunction(
      [this](QString text) { m_fmdl_header.fshp_count = text.toUShort(); });
  header_attributes_model->setItem(row, 1, fshp_count_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // FMAT Count
  header_attributes_model->setItem(row, 0, new QStandardItem("FMAT Count"));
  CustomStandardItem* fmat_count_item =
      new CustomStandardItem(QString::number(m_fmdl_header.fmat_count));
  fmat_count_item->SetFunction(
      [this](QString text) { m_fmdl_header.fmat_count = text.toUShort(); });
  header_attributes_model->setItem(row, 1, fmat_count_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // User Data Entry Count
  header_attributes_model->setItem(row, 0, new QStandardItem("User Data Entry Count"));
  CustomStandardItem* user_data_entry_count_item =
      new CustomStandardItem(QString::number(m_fmdl_header.user_data_entry_count));
  user_data_entry_count_item->SetFunction(
      [this](QString text) { m_fmdl_header.user_data_entry_count = text.toUShort(); });
  header_attributes_model->setItem(row, 1, user_data_entry_count_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Number of Verticies
  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Vertices"));
  CustomStandardItem* num_vertices_item =
      new CustomStandardItem(QString::number(m_fmdl_header.num_vertices));
  num_vertices_item->SetFunction(
      [this](QString text) { m_fmdl_header.num_vertices = text.toUShort(); });
  header_attributes_model->setItem(row, 1, num_vertices_item);
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // User Runtime Pointer
  header_attributes_model->setItem(row, 0, new QStandardItem("User Runtime Pointer"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_fmdl_header.user_pointer_runtime, 16)));
  ++row;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  connect(header_attributes_model, &QStandardItemModel::itemChanged, this,
          &FMDLNode::HandleAttributeItemChange);

  emit NewAttributesArea(MakeAttributeSection(header_attributes_model));
  return ResultCode::Success;
}

void FMDLNode::HandleAttributeItemChange(QStandardItem* item)
{
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;

  CustomStandardItem* custom_item = dynamic_cast<CustomStandardItem*>(item);
  if (custom_item)
    custom_item->ExecuteFunction();

  m_fmdl.SetHeader(m_fmdl_header);
  emit NewFMDL(m_fmdl);
}
