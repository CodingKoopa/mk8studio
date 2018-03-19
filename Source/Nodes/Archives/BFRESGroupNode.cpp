#include "Nodes/Archives/BFRESGroupNode.h"

#include "Nodes/Models/FMDLNode.h"
#include "Nodes/Textures/FTEXNode.h"

template <typename GroupType>
BFRESGroupNode<GroupType>::BFRESGroupNode(std::shared_ptr<ResourceDictionary<GroupType>> dictionary,
                                          QObject* parent)
    : Node(parent), m_dictionary(dictionary)
{
}

template <typename GroupType>
CustomStandardItem* BFRESGroupNode<GroupType>::MakeItem()
{
  if (!m_header_loaded)
  {
    ResultCode res = m_dictionary->ReadHeader();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return nullptr;
    }
    else
    {
      m_dictionary_header = m_dictionary->GetHeader();
      m_header_loaded = true;
    }
  }
  if (!m_nodes_loaded)
  {
    ResultCode res = m_dictionary->ReadNodes();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return nullptr;
    }
    else
      // There's no node list or anything to populate, the nodes are accessed directly with [].
      m_nodes_loaded = true;
  }

  CustomStandardItem* item = MakeGroupDependentItem();

  item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  return item;
}

template <>
CustomStandardItem* BFRESGroupNode<FMDL>::MakeGroupDependentItem()
{
  CustomStandardItem* group_node = new CustomStandardItem("FMDL Models");

  // Skip the root node by starting at 1.
  for (quint32 row = 1; row < m_dictionary->Size(); ++row)
  {
    FMDLNode* fmdl_node = new FMDLNode((*m_dictionary)[row].value, this);
    connect(fmdl_node, &FMDLNode::ConnectNode, this, &BFRESGroupNode::ConnectNode);
    emit ConnectNode(fmdl_node);
    group_node->appendRow(fmdl_node->MakeItem());
  }

  return group_node;
}

template <>
CustomStandardItem* BFRESGroupNode<FTEX>::MakeGroupDependentItem()
{
  CustomStandardItem* group_node = new CustomStandardItem("FTEX Textures");

  // Skip the root node by starting at 1.
  for (quint32 row = 1; row < m_dictionary->Size(); ++row)
  {
    FTEXNode* ftex_node = new FTEXNode((*m_dictionary)[row].value, this);
    // Sync the FMDL object.
    //  connect(child_node, &FMDLNode::NewFMDL, this, [this, row](const FMDL& fmdl) {
    //    m_bfres.SetFMDL(fmdl, row);
    //    emit NewNodeList(this->);
    //  });
    connect(ftex_node, &FTEXNode::ConnectNode, this, &BFRESGroupNode::ConnectNode);
    emit ConnectNode(ftex_node);
    group_node->appendRow(ftex_node->MakeItem());
  }

  return group_node;
}

template <typename GroupType>
ResultCode BFRESGroupNode<GroupType>::LoadAttributeArea()
{
  if (!m_header_loaded)
  {
    ResultCode res = m_dictionary->ReadHeader();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return res;
    }
    else
    {
      m_dictionary_header = m_dictionary->GetHeader();
      m_header_loaded = true;
    }
  }
  if (!m_nodes_loaded)
  {
    ResultCode res = m_dictionary->ReadNodes();
    if (res != ResultCode::Success)
    {
      emit NewStatus(res);
      return res;
    }
    else
      // There's no node list or anything to populate, the nodes are accessed directly with [].
      m_nodes_loaded = true;
  }

  m_delegate_group = CustomItemDelegate::DelegateGroup();

  QStandardItemModel* group_attributes_model = new QStandardItemModel();

  int row = 0;

  // Size (In bytes)
  group_attributes_model->setItem(row, 0, new QStandardItem("Size"));
  group_attributes_model->setItem(row, 1,
                                  new QStandardItem(QString::number(m_dictionary_header.size)));
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Number of nodes (Files)
  group_attributes_model->setItem(row, 0, new QStandardItem("Number of nodes"));
  group_attributes_model->setItem(
      row, 1, new QStandardItem(QString::number(m_dictionary_header.num_nodes)));
  m_delegate_group.spin_box_delegates << row;
  ++row;

  group_attributes_model->setRowCount(row);
  group_attributes_model->setColumnCount(2);

  emit NewAttributesArea(MakeAttributeSection(group_attributes_model));
  return ResultCode::Success;
}

template <typename GroupType>
void BFRESGroupNode<GroupType>::SetDictionary(std::shared_ptr<ResourceDictionary<GroupType>> value)
{
  m_dictionary = value;
}

// The values aren't meant to be changed.
template <typename GroupType>
void BFRESGroupNode<GroupType>::HandleAttributeItemChange(QStandardItem*)
{
  return;
}
