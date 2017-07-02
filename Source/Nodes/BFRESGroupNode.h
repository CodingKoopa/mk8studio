#ifndef BFRESGROUPNODE_H
#define BFRESGROUPNODE_H

#include "Formats/BFRES.h"
#include "Node.h"

class BFRESGroupNode : public Node
{
  Q_OBJECT
public:
  explicit BFRESGroupNode(quint32 group, const BFRES& bfres, const QVector<BFRES::Node*>& node_list,
                          QObject* parent = 0);

  QStandardItem* MakeItem();
  QStandardItem* MakeListItemFromRawList();
  QStandardItem* MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node = -1);
  ResultCode LoadAttributeArea();

private:
  quint32 m_group;
  BFRES m_bfres;
  BFRES::Header m_bfres_header;

  QVector<BFRES::Node*> m_node_list;
  QVector<int> m_node_blacklist;

private slots:
  void HandleAttributeItemChange(QStandardItem* item);
};

Q_DECLARE_METATYPE(BFRESGroupNode*)

#endif  // BFRESGROUPNODE_H
