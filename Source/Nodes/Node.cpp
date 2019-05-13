#include "Nodes/Node.h"

#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QTableView>

#include "Nodes/Archives/BFRESGroupNode.h"
#include "Nodes/Archives/BFRESNode.h"
#include "Nodes/Models/FMDLNode.h"
#include "Nodes/Models/FVTXAttributeNode.h"
#include "Nodes/Models/FVTXBufferNode.h"
#include "Nodes/Models/FVTXNode.h"
#include "Nodes/Textures/FTEXNode.h"

Node::Node(QObject* parent)
    : QObject(parent), m_tree_view(nullptr), m_main_widget(nullptr), m_context_menu(nullptr)
{
}

DynamicStandardItem* Node::MakeLabelItem(QString label)
{
  DynamicStandardItem* attribute_item = new DynamicStandardItem;
  attribute_item->setData(label, Qt::DisplayRole);
  attribute_item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  return attribute_item;
}

QScrollArea* Node::MakeAttributeSection(const QVector<Node::AttributeTableRow>& table_rows,
                                        std::function<void()> update_data)
{
  QTableWidget* table_widget = new QTableWidget(table_rows.size(), 2);

  for (qint32 table_row = 0; table_row < table_rows.size(); ++table_row)
  {
    QTableWidgetItem* name_item = new QTableWidgetItem(table_rows[table_row].attribute_name);
    name_item->setFlags(name_item->flags() ^ Qt::ItemIsEditable);
    table_widget->setItem(table_row, 0, name_item);

    QTableWidgetItem* value_item = new QTableWidgetItem;
    if (!table_rows[table_row].editable)
      value_item->setFlags(value_item->flags() ^ Qt::ItemIsEditable);
    value_item->setData(Qt::UserRole, QVariant::fromValue(table_rows[table_row].handle_edit));

    // Handle special cases for attribute values.

    // Combo box attributes.
    if (table_rows[table_row].attribute_value.canConvert<AttributeComboBoxData>())
    {
      AttributeComboBoxData attribute_combo_box_data =
          table_rows[table_row].attribute_value.value<AttributeComboBoxData>();
      value_item->setData(Qt::DisplayRole,
                          attribute_combo_box_data.first[attribute_combo_box_data.second]);

      QComboBox* attribute_combo_box = new QComboBox;
      for (const auto& attribute_value_name : attribute_combo_box_data.first)
        attribute_combo_box->addItem(attribute_value_name);
      attribute_combo_box->setCurrentIndex(attribute_combo_box_data.second);
      table_widget->setCellWidget(table_row, 1, attribute_combo_box);
    }
    // Other attributes.
    else
    {
      value_item->setData(Qt::DisplayRole, table_rows[table_row].attribute_value);
    }
  }
  table_widget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_widget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_widget->horizontalHeader()->hide();
  table_widget->verticalHeader()->hide();

  connect(table_widget, &QTableWidget::itemChanged, [update_data](QTableWidgetItem* item) {
    if (HandleEditFunction handle_edit = item->data(Qt::UserRole).value<HandleEditFunction>())
      handle_edit(item->data(Qt::EditRole));
    if (update_data)
      update_data();
  });

  //  QVector<Node::AttributeTableRow> test{
  //      {"Magic File Identifier", m_bfres_header.magic},
  //      {"Unknown A", QString("0x" + QString::number(m_bfres_header.unknown_a, 16))},
  //      {"Unknown B", QString("0x" + QString::number(m_bfres_header.unknown_b, 16))},
  //      {"Unknown C", QString("0x" + QString::number(m_bfres_header.unknown_c, 16))},
  //      {"Unknown D", QString("0x" + QString::number(m_bfres_header.unknown_d, 16))},
  //      {"Unknown E", QString("0x" + QString::number(m_bfres_header.unknown_e, 16))},
  //      {"Unknown F", QString("0x" + QString::number(m_bfres_header.unknown_f, 16))},
  //      {"Endianess", QVariant::fromValue(Node::AttributeComboBoxData{[this] {
  //         QVector<QString> endian_names_vector;
  //         auto endian_name_map = m_bfres->GetEndianNames();
  //         for (auto const& endian_name : endian_name_map)
  //           endian_names_vector << endian_name.second;

  //         // The usual std::distance way of selecting an index doesn't work here, possibly
  //         because of
  //         // it being a 2 element map. This is a bit of a hack.
  //         return Node::AttributeComboBoxData(
  //             endian_names_vector,
  //             static_cast<BFRES::Endianness>(m_bfres_header.bom) == BFRES::Endianness::Little ? 0
  //             :
  //                                                                                               1);
  //       }()})}};

  // To have all editors open by default, uncomment this out
  // PROS: Looks nicer, possibly more convienient
  // CONS: Scrolling can accidentally change values, and opening a new sections
  // seems to select the text
  // in every open editor for some reason
  // for (int i = 0; i < sectionHeaderModel->rowCount(); i++)
  // tableView->openPersistentEditor(sectionHeaderModel->index(i, 1));

  QVBoxLayout* attributes_layout = new QVBoxLayout();
  attributes_layout->addWidget(new QLabel("Header"));
  attributes_layout->addWidget(table_widget);

  QScrollArea* section_container = new QScrollArea();
  section_container->setLayout(attributes_layout);

  return section_container;
}

QScrollArea* Node::MakeAttributeSectionOld(QStandardItemModel* table_view_layout)
{
  QTableView* table_view = new QTableView;

  table_view->setModel(table_view_layout);
  // stretch out table to fit space
  table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_view->verticalHeader()->hide();
  table_view->horizontalHeader()->hide();

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

  else if (FVTXBufferNode* fvtx_buffer_node = dynamic_cast<FVTXBufferNode*>(upcast))
    fvtx_buffer_node->LoadAttributeArea();

  // TODO: dunno if there's any way to not have to specify a type?
  else if (BFRESGroupNode<FMDL>* bfres_group_node = dynamic_cast<BFRESGroupNode<FMDL>*>(upcast))
    bfres_group_node->LoadAttributeArea();

  else if (BFRESGroupNode<FTEX>* bfres_group_node = dynamic_cast<BFRESGroupNode<FTEX>*>(upcast))
    bfres_group_node->LoadAttributeArea();
}

void Node::HandleTreeCustomContextMenuRequest(const QPoint& point)
{
  QModelIndex index = m_tree_view->indexAt(point);

  if (Node* node = qvariant_cast<Node*>(index.data(Qt::UserRole + 1)))
  {
    if (node->m_context_menu)
      node->m_context_menu->exec(m_tree_view->mapToGlobal(point));
  }
}