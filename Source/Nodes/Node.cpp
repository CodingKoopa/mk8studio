#include "Node.h"

#include <QMenu>

#include "BFRESGroupNode.h"
#include "BFRESNode.h"
#include "FTEXNode.h"

QMenu* Node::GetContextMenu()
{
  return m_context_menu;
}

void Node::HandleFileTreeClick(QModelIndex index)
{
  if (BFRESNode* bfres_node = qvariant_cast<BFRESNode*>(index.data(Qt::UserRole + 1)))
    bfres_node->LoadAttributeArea();

  else if (BFRESGroupNode* bfres_group_node =
               qvariant_cast<BFRESGroupNode*>(index.data(Qt::UserRole + 1)))
    bfres_group_node->LoadAttributeArea();

  else if (FTEXNode* ftex_node = qvariant_cast<FTEXNode*>(index.data(Qt::UserRole + 1)))
  {
    ftex_node->LoadAttributeArea();
    ftex_node->LoadMainWidget();
  }
}

void Node::HandleTreeCustomContextMenuRequest(const QPoint& point)
{
  QModelIndex index = m_tree_view->indexAt(point);

  if (Node* node = qvariant_cast<Node*>(index.data(Qt::UserRole + 1)))
  {
    QMenu* context_menu = node->GetContextMenu();
    if (context_menu)
      context_menu->exec(m_tree_view->mapToGlobal(point));
  }
}
