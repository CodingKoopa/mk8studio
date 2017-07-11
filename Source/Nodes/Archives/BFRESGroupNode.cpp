#include "BFRESGroupNode.h"

#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>

#include "CustomDelegate.h"
#include "Nodes/Models/FMDLNode.h"
#include "Nodes/Textures/FTEXNode.h"

BFRESGroupNode::BFRESGroupNode(quint32 group, const BFRES& bfres,
                               const QVector<BFRES::Node*>& node_list, QObject* parent)
    : Node(parent), m_group(group), m_bfres(bfres), m_bfres_header(m_bfres.GetHeader()),
      m_node_list(node_list)
{
}

CustomStandardItem* BFRESGroupNode::MakeItem()
{
  CustomStandardItem* item;
  // TODO: Make this a configurable option, add in MakeTreeItemFromSubtree.
  bool use_bst = false;
  if (!use_bst)
    item = MakeListItemFromRawList();
  item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  return item;
}

CustomStandardItem* BFRESGroupNode::MakeListItemFromRawList()
{
  CustomStandardItem* item = new CustomStandardItem();
  item->setData(QString("Group " + QString::number(m_group)), Qt::DisplayRole);
  // Skip the first node (Dummy root node.).
  for (int row = 1; row < m_node_list.size(); ++row)
  {
    CustomStandardItem* child_item = new CustomStandardItem;
    switch (static_cast<BFRES::GroupType>(m_group))
    {
    case BFRES::GroupType::FMDL:
    {
      FMDL fmdl(m_bfres.GetFile(), m_bfres.GetRawNodeLists()[m_group][row]->data_ptr);
      fmdl.SetName(m_bfres.GetRawNodeLists()[m_group][row]->name);
      FMDLNode* child_node = new FMDLNode(fmdl, this);
      connect(child_node, &FMDLNode::ConnectNode, this, &BFRESGroupNode::ConnectNode);
      emit ConnectNode(child_node);
      child_item = child_node->MakeItem();
      break;
    }
    case BFRES::GroupType::FTEX:
    {
      FTEX ftex(m_bfres.GetFile(), m_bfres.GetRawNodeLists()[m_group][row]->data_ptr);
      ftex.SetName(m_bfres.GetRawNodeLists()[m_group][row]->name);
      FTEXNode* child_node = new FTEXNode(ftex, this);
      connect(child_node, &FTEXNode::ConnectNode, this, &BFRESGroupNode::ConnectNode);
      emit ConnectNode(child_node);
      child_item = child_node->MakeItem();
      break;
    }
    default:
      child_item->setData(m_node_list[row]->name, Qt::DisplayRole);
      child_item->setData(QVariant::fromValue<Node*>(nullptr), Qt::UserRole + 1);
      break;
    }
    item->appendRow(child_item);
  }
  return item;
}

CustomStandardItem* BFRESGroupNode::MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node)
{
  if (node)
  {
    CustomStandardItem* item = new CustomStandardItem();
    item->setData(node->name, Qt::DisplayRole);
    if (!m_node_blacklist.contains(node->left_index))
    {
      // TODO: use the better way in BFRES class
      if (blacklist_node != -1)
        m_node_blacklist.append(blacklist_node);

      QStandardItem* left_node_item = MakeTreeItemFromSubtree(node->left_node, node->left_index);
      if (left_node_item)
        item->appendRow(left_node_item);
    }
    if (!m_node_blacklist.contains(node->right_index))
    {
      if (blacklist_node != -1)
        m_node_blacklist.append(blacklist_node);

      QStandardItem* right_node_item = MakeTreeItemFromSubtree(node->right_node, node->right_index);
      if (right_node_item)
        item->appendRow(right_node_item);
    }
    return item;
  }
  else
    return nullptr;
}

ResultCode BFRESGroupNode::LoadAttributeArea()
{
  m_delegate_group = CustomItemDelegate::DelegateGroup();

  QStandardItemModel* group_attributes_model = new QStandardItemModel();

  int row = 0;

  group_attributes_model->setItem(row, 0, new QStandardItem("Offset"));
  group_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_offsets[m_group], 16)));
  ++row;

  group_attributes_model->setItem(row, 0, new QStandardItem("Number of files"));
  group_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_counts[m_group], 16)));
  m_delegate_group.spin_box_delegates.append(1);

  group_attributes_model->setRowCount(row);
  group_attributes_model->setColumnCount(2);

  QTableView* table_view = new QTableView;
  table_view->setModel(group_attributes_model);
  // stretch out table to fit space
  table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_view->verticalHeader()->hide();
  table_view->horizontalHeader()->hide();

  CustomItemDelegate* customDelegate = new CustomItemDelegate(m_delegate_group);
  table_view->setItemDelegate(customDelegate);

  QVBoxLayout* sections_layout = new QVBoxLayout();
  sections_layout->addWidget(new QLabel("Group " + QString::number(m_group)));
  sections_layout->addWidget(table_view);

  QScrollArea* attributes_container = new QScrollArea();
  attributes_container->setLayout(sections_layout);

  emit NewAttributesArea(attributes_container);
  return ResultCode::Success;
}

void BFRESGroupNode::HandleAttributeItemChange(QStandardItem* item)
{
  // TODO. See: BFRESNode::HandleAttributeItemChange
  item = item;
}
