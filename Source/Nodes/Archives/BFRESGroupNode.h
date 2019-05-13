#pragma once

#include <QHeaderView>

#include "Formats/Archives/BFRES.h"
#include "Formats/Common/ResourceDictionary.h"
#include "Formats/Models/FMDL.h"
#include "Nodes/Models/FMDLNode.h"
#include "Nodes/Node.h"
#include "Nodes/Textures/FTEXNode.h"
#include "QtUtils/DynamicStandardItem.h"

template <typename GroupType>
class BFRESGroupNode : public Node
{
public:
  BFRESGroupNode(std::shared_ptr<ResourceDictionary<GroupType>> dictionary = {},
                 QObject* parent = 0)
      : Node(parent), m_dictionary(dictionary)
  {
  }

  DynamicStandardItem* MakeGroupDependentItem();

  DynamicStandardItem* MakeItem()
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

    DynamicStandardItem* item = MakeGroupDependentItem();

    item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
    emit ConnectNode(this);
    return item;
  }

  ResultCode LoadAttributeArea()
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

    emit NewAttributeArea(MakeAttributeSection(
        {{"Size", m_dictionary_header.size}, {"Number of nodes", m_dictionary_header.num_nodes}}));
    return ResultCode::Success;
  }

  void SetDictionary(std::shared_ptr<ResourceDictionary<GroupType>> value) { m_dictionary = value; }

private:
  std::shared_ptr<ResourceDictionary<GroupType>> m_dictionary;
  // TODO: what's this? owo
  typename ResourceDictionary<GroupType>::Header m_dictionary_header;

  bool m_header_loaded = false;
  bool m_nodes_loaded = false;
};
