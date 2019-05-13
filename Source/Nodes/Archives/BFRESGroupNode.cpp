#include "Nodes/Archives/BFRESGroupNode.h"

template <>
DynamicStandardItem* BFRESGroupNode<FMDL>::MakeGroupDependentItem()
{
  DynamicStandardItem* group_node = new DynamicStandardItem("FMDL Models");

  // Skip the root node by starting at 1.
  for (quint32 row = 1; row < m_dictionary->Size(); ++row)
  {
    FMDLNode* fmdl_node = new FMDLNode((*m_dictionary)[row].value, this);
    connect(fmdl_node, &FMDLNode::ConnectNode, this, &BFRESGroupNode::ConnectNode);
    group_node->appendRow(fmdl_node->MakeItem());
  }

  return group_node;
}

template <>

DynamicStandardItem* BFRESGroupNode<FTEX>::MakeGroupDependentItem()
{
  DynamicStandardItem* group_node = new DynamicStandardItem("FTEX Textures");

  // Skip the root node by starting at 1.
  for (quint32 row = 1; row < m_dictionary->Size(); ++row)
  {
    FTEXNode* ftex_node = new FTEXNode((*m_dictionary)[row].value, this);
    connect(ftex_node, &FTEXNode::ConnectNode, this, &BFRESGroupNode::ConnectNode);
    group_node->appendRow(ftex_node->MakeItem());
  }

  return group_node;
}