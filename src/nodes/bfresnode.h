#ifndef BFRESGUI_H
#define BFRESGUI_H

#include <QScrollArea>
#include <QStandardItem>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "customdelegate.h"
#include "formats/bfres.h"
#include "ftexnode.h"

class BFRESNode : public Node
{
  Q_OBJECT
public:
  explicit BFRESNode(BFRES m_bfres, QObject* parent = 0);
  // TODO: make a resultcode enum
  ResultCode LoadAttributeArea() override;
  ResultCode LoadFileTreeArea() override;
  ResultCode LoadGroupAttributeArea(int groupNum);

private:
  QStandardItem* MakeListItemFromRawList(QVector<BFRES::Node*> list, BFRES::GroupType group);
  QStandardItem* MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node = -1);

  BFRES m_bfres;
  BFRES::BFRESHeader m_bfres_header;

  // TODO: might not be necessary

  CustomDelegate::DelegateGroup m_delegate_group;

  QScrollArea* m_file_tree_container;
  QVBoxLayout* m_file_tree_layout;

  QWidget* mainWidget;

  QVector<int> m_node_blacklist;

  enum NodeType
  {
    NODE_ROOT_ITEM,
    NODE_BFRES_GROUP,
    NODE_FTEX,
    NODE_NONE
  };

private slots:
  void HandleAttributeItemChange(QStandardItem* item);
  void handleFileTreeClick(QModelIndex index);
};

#endif  // BFRESGUI_H
