#pragma once

#include <QHeaderView>

#include "CustomStandardItem.h"
#include "Formats/Archives/BFRES.h"
#include "Formats/Common/ResourceDictionary.h"
#include "Formats/Models/FMDL.h"
#include "Nodes/Node.h"

class BFRESGroupNodeQObject : public Node
{
  Q_OBJECT
public:
  BFRESGroupNodeQObject(QObject* parent = 0);

signals:
  void NewDictionary(const ResourceDictionary<FMDL>&);
  void NewDictionary(const ResourceDictionary<FTEX>&);

protected slots:
  void HandleAttributeItemChange(QStandardItem* item);
};

template <typename GroupType>
class BFRESGroupNode : public BFRESGroupNodeQObject
{
public:
  BFRESGroupNode(const ResourceDictionary<GroupType>& dictionary = ResourceDictionary<GroupType>(),
                 QObject* parent = 0);
  BFRESGroupNode(const BFRESGroupNode& other);
  BFRESGroupNode& operator=(const BFRESGroupNode& other);

  CustomStandardItem* MakeItem();

  CustomStandardItem* MakeGroupDependentItem();

  ResultCode LoadAttributeArea();

  void SetDictionary(const ResourceDictionary<GroupType>& value);

private:
  ResourceDictionary<GroupType> m_dictionary;
  // TODO: what's this? owo
  typename ResourceDictionary<GroupType>::Header m_dictionary_header;

  bool m_header_loaded = false;
  bool m_nodes_loaded = false;
};

// Unlike ResourceDictionary class, there an only be groups for each of the 12 groups, which is
// easier to keep track of. By explicitly initializing instances of this class here, we can not only
// keep the definitions in a separate implementation file, but also selectively pick only valid
// groups that have specializations.
template class BFRESGroupNode<FMDL>;
template class BFRESGroupNode<FTEX>;
