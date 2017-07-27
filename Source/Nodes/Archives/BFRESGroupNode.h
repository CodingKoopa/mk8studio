#pragma once

#include "CustomStandardItem.h"
#include "Formats/Archives/BFRES.h"
#include "Nodes/Node.h"

class BFRESGroupNode : public Node
{
  Q_OBJECT
public:
  explicit BFRESGroupNode(quint32 group, const BFRES& bfres, const QVector<BFRES::Node*>& node_list,
                          QObject* parent = 0);

  CustomStandardItem* MakeItem();
  CustomStandardItem* MakeListItemFromRawList();
  CustomStandardItem* MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node = -1);
  ResultCode LoadAttributeArea();

private:
  quint32 m_group = 0;
  // TODO: might not be necessary
  BFRES m_bfres = BFRES(nullptr);
  BFRES::Header m_bfres_header = BFRES::Header();

  QVector<BFRES::Node*> m_node_list = QVector<BFRES::Node*>();
  QVector<int> m_node_blacklist = QVector<int>();

private slots:
  void HandleAttributeItemChange(QStandardItem* item);
};
