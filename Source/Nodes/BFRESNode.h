#ifndef BFRESGUI_H
#define BFRESGUI_H

#include <QScrollArea>
#include <QStandardItem>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "CustomDelegate.h"
#include "FTEXNode.h"
#include "Formats/BFRES.h"

class BFRESNode : public Node
{
  Q_OBJECT
public:
  explicit BFRESNode(BFRES* bfres, QObject* parent = 0) : Node(parent), m_bfres(bfres) {}
  ~BFRESNode() { delete m_bfres; }
  ResultCode LoadFileTreeArea() override;
  ResultCode LoadAttributeArea() override;
  QStandardItem* MakeItem() override;

private:
  QStandardItem* MakeListItemFromRawList(QVector<BFRES::Node*> list, BFRES::GroupType group);
  QStandardItem* MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node = -1);

  BFRES* m_bfres;
  BFRES::BFRESHeader m_bfres_header;

  QScrollArea* m_file_tree_container;
  QVBoxLayout* m_file_tree_layout;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};

// Make this class known to the template-based functions to allow casting to and from a QVariant.
Q_DECLARE_METATYPE(BFRESNode*)

#endif  // BFRESGUI_H
