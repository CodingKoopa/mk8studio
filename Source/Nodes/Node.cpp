#include "Node.h"

#include <QMenu>

#include "BFRESGroupNode.h"
#include "BFRESNode.h"
#include "FTEXNode.h"

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

  if (FTEXNode* ftex_node = qvariant_cast<FTEXNode*>(index.data(Qt::UserRole + 1)))
  {
    QMenu context_menu;
    QAction* action_export = new QAction("Export");
    connect(action_export, SIGNAL(triggered()), ftex_node, SLOT(HandleExportActionClick()));
    context_menu.addAction(action_export);
    context_menu.exec(m_tree_view->mapToGlobal(point));
  }
}
