#include "Nodes/Node.h"

#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QTableView>

#include "Nodes/Archives/BFRESGroupNode.h"
#include "Nodes/Archives/BFRESNode.h"
#include "Nodes/Models/FMDLNode.h"
#include "Nodes/Models/FVTXAttributeNode.h"
#include "Nodes/Models/FVTXNode.h"
#include "Nodes/Textures/FTEXNode.h"

Node::Node(QObject* parent)
    : QObject(parent), m_tree_view(nullptr), m_main_widget(nullptr), m_context_menu(nullptr)
{
}

QMenu* Node::GetContextMenu()
{
  return m_context_menu;
}

QScrollArea* Node::MakeAttributeSection(QStandardItemModel* table_view_layout)
{
  QTableView* table_view = new QTableView;

  table_view->setModel(table_view_layout);
  // stretch out table to fit space
  table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_view->verticalHeader()->hide();
  table_view->horizontalHeader()->hide();

  table_view->setItemDelegate(new CustomItemDelegate(m_delegate_group));

  // To have all editors open by default, uncomment this out
  // PROS: Looks nicer, possibly more convienient
  // CONS: Scrolling can accidentally change values, and opening a new sections
  // seems to select the text
  // in every open editor for some reason
  // for (int i = 0; i < sectionHeaderModel->rowCount(); i++)
  // tableView->openPersistentEditor(sectionHeaderModel->index(i, 1));

  QVBoxLayout* attributes_layout = new QVBoxLayout();
  attributes_layout->addWidget(new QLabel("Header"));
  attributes_layout->addWidget(table_view);

  QScrollArea* container = new QScrollArea();
  container->setLayout(attributes_layout);

  return container;
}

void Node::HandleFileTreeClick(QModelIndex index)
{
  Node* upcast = index.data(Qt::UserRole + 1).value<Node*>();

  // Normal Classes

  if (BFRESNode* bfres_node = dynamic_cast<BFRESNode*>(upcast))
    bfres_node->LoadAttributeArea();

  else if (FMDLNode* fmdl_node = dynamic_cast<FMDLNode*>(upcast))
    fmdl_node->LoadAttributeArea();

  else if (FVTXNode* fvtx_node = dynamic_cast<FVTXNode*>(upcast))
    fvtx_node->LoadAttributeArea();

  else if (FTEXNode* ftex_node = dynamic_cast<FTEXNode*>(upcast))
  {
    ftex_node->LoadAttributeArea();
    ftex_node->LoadMainWidget();
  }

  else if (FVTXAttributeNode* fvtx_attribute_node = dynamic_cast<FVTXAttributeNode*>(upcast))
    fvtx_attribute_node->LoadAttributeArea();

  // TODO: dunno if there's any way to not have to specify a type?
  else if (BFRESGroupNode<FMDL>* bfres_group_node = dynamic_cast<BFRESGroupNode<FMDL>*>(upcast))
    bfres_group_node->LoadAttributeArea();
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
