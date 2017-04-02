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
  ResultCode SaveFile() override;

private:
  QStandardItem* MakeListItemFromRawList(QVector<BFRES::Node*> list, int group);
  QStandardItem* MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node = -1);

  BFRES m_bfres;
  BFRES::BFRESHeader m_bfres_header;

  CustomDelegate::delegateGroup_t m_delegate_group;

  // *******
  // TODO this shouldn't be a member variable
  // (just a note to myself because it's like almost 2 sooo)
  QTableView* tableView;
  QTreeView* treeView;

  QScrollArea* m_file_tree_container;
  QVBoxLayout* m_file_tree_layout;

  QWidget* mainWidget;

  QVector<int> m_node_blacklist;

  enum entryType_t
  {
    ROOT_ITEM,
    BFRES_GROUP,
    FTEX_NODE
  };

private slots:
  void HandleAttributeItemChange(QStandardItem* item);
  void handleFileTreeClick(QModelIndex index);
};

#endif  // BFRESGUI_H
