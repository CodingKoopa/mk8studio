#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QObject>
#include <QStandardItem>
#include <QTableView>

#include "common.h"
#include "customdelegate.h"
#include "imageview.h"

#include "bfresnode.h"

BFRESNode::BFRESNode(BFRES bfres, QObject* parent) : Node(parent), m_bfres(bfres)
{
}

ResultCode BFRESNode::LoadFileTreeArea()
{
  // This doesn't seem like the right place to put this, but the ctor doesn't work
  emit ConnectNode(this);

  emit NewStatus(RESULT_STATUS_BAR_UPDATE, "Loading file tree...");

  ResultCode res = m_bfres.ReadHeader();
  if (res != RESULT_SUCCESS)
  {
    NewStatus(res);
    return res;
  }
  m_bfres_header = m_bfres.getHeader();
  // TODO: have this return a result code
  m_bfres.ReadIndexGroups();
  // QVector<BFRES::Node*> root_nodes = bfres.GetRootNodes();
  QVector<QVector<BFRES::Node*>> raw_node_list = m_bfres.GetRawNodeLists();

  QStandardItemModel* fileTreeModel = new QStandardItemModel(0, 1);

  QStandardItem* rootItem = new QStandardItem(QString(m_bfres_header.file_name + " (BFRES)"));
  rootItem->setData(NODE_ROOT_ITEM, Qt::UserRole + 1);
  for (int group = 0; group < m_bfres_header.file_counts.size(); group++)
  {
    if (m_bfres_header.file_counts[group] != 0)
    {
      // For BST testing, use this:
      //      if (m_bfres.GetRootNodes()[group])
      //      {
      //        QStandardItem* group_item = MakeTreeItemFromSubtree(m_bfres.GetRootNodes()[group]);
      //        m_node_blacklist.clear();
      QStandardItem* group_item =
          MakeListItemFromRawList(raw_node_list[group], (BFRES::GroupType)group);
      if (group_item)
      {
        // the string to be displayed
        group_item->setData("Group " + QString::number(group), Qt::DisplayRole);
        // the type of item
        group_item->setData(NODE_BFRES_GROUP, Qt::UserRole + 1);
        // the data itself
        group_item->setData(group, Qt::UserRole + 2);

        rootItem->appendRow(group_item);
      }
      //      }
    }
  }
  fileTreeModel->appendRow(rootItem);

  QTreeView* tree_view = new QTreeView;
  // stretch out table to fit space
  tree_view->header()->hide();
  // there might be a right click menu delegate here in the (probably far)
  // future
  tree_view->setItemDelegate(new CustomDelegate(CustomDelegate::DelegateGroup()));
  tree_view->setModel(fileTreeModel);
  // todo: this only works with mouse clicking, i still have to figure out
  // keyboard arrow navigation
  connect(tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(handleFileTreeClick(QModelIndex)));
  connect(tree_view, SIGNAL(doubleClicked(QModelIndex)), this,
          SLOT(handleFileTreeClick(QModelIndex)));
  m_file_tree_container = new QScrollArea();

  m_file_tree_layout = new QVBoxLayout();
  m_file_tree_layout->addWidget(tree_view);

  m_file_tree_container = new QScrollArea();
  m_file_tree_container->setLayout(m_file_tree_layout);

  emit NewFileTreeArea(m_file_tree_container);

  emit NewStatus(res);
  return res;
}

ResultCode BFRESNode::LoadAttributeArea()
{
  emit NewStatus(RESULT_STATUS_BAR_UPDATE, "Loading file info...");

  QStandardItemModel* header_attributes_model = new QStandardItemModel();
  m_delegate_group = CustomDelegate::DelegateGroup();

  int row = 0;

  // Magic
  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  QStandardItem* magicValueItem = new QStandardItem(m_bfres_header.magic);
  header_attributes_model->setItem(row, 1, magicValueItem);
  m_delegate_group.line_edit_delegates << row;
  row++;

  // Unknown A
  header_attributes_model->setItem(row, 0, new QStandardItem("Unknown 1"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem(QString("0x" + QString::number(m_bfres_header.unknown_a, 16))));
  m_delegate_group.spin_box_delegates << row;
  row++;
  // TODO: the other unknowns

  // Endianess
  header_attributes_model->setItem(row, 0, new QStandardItem("Endianess"));
  QStandardItemModel* endian_entries = new QStandardItemModel(2, 0);
  // Make a list of possible entries for the endianness
  endian_entries->setItem(0, 0, new QStandardItem("Big"));
  endian_entries->setItem(1, 0, new QStandardItem("Little"));
  m_delegate_group.combo_box_entries << endian_entries;
  m_delegate_group.combo_box_delegates << row;

  QStandardItem* endianness_item = new QStandardItem();

  if (m_bfres_header.bom == 0xFEFF)
  {
    // Visually set text shown without the combobox activated.
    endianness_item->setText("Big");
    // Internally set index
    m_delegate_group.combo_box_selections << 0;
  }
  else if (m_bfres_header.bom == 0xFFFE)
  {
    endianness_item->setText("Little");
    m_delegate_group.combo_box_selections << 1;
  }
  else
  {
    NewStatus(RESULT_BFRES_ENDIANNESS);
    return RESULT_BFRES_ENDIANNESS;
  }
  header_attributes_model->setItem(row, 1, endianness_item);
  row++;

  // Length
  header_attributes_model->setItem(row, 0, new QStandardItem("Length"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem(QString::number(m_bfres_header.length)));
  m_delegate_group.spin_box_delegates << row;
  row++;

  // Alignment
  header_attributes_model->setItem(row, 0, new QStandardItem("Alignment"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.alignment, 16)));
  row++;

  // File name offset
  header_attributes_model->setItem(row, 0, new QStandardItem("File Name Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_name_offset, 16)));
  // TODO: hex spinbox delegate maybe?
  row++;

  // String table length
  header_attributes_model->setItem(row, 0, new QStandardItem("String Table Length"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.string_table_length, 16)));
  row++;

  // String table offset
  header_attributes_model->setItem(row, 0, new QStandardItem("String Table Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.string_table_offset, 16)));
  row++;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  QObject::connect(header_attributes_model, SIGNAL(itemChanged(QStandardItem*)), this,
                   SLOT(HandleAttributeItemChange(QStandardItem*)));

  // at this point, the model is ready to go, and be put into a view

  QTableView* table_view = new QTableView();

  table_view->setModel(header_attributes_model);
  // stretch out table to fit space
  table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_view->verticalHeader()->hide();
  table_view->horizontalHeader()->hide();

  table_view->setItemDelegate(new CustomDelegate(m_delegate_group));

  // To have all editors open by default, uncomment this out
  // PROS: Looks nicer, possibly more convienient
  // CONS: Scrolling can accidentally change values, and opening a new sections
  // seems to select the text
  // in every open editor for some reason
  // for (int i = 0; i < sectionHeaderModel->rowCount(); i++)
  // tableView->openPersistentEditor(sectionHeaderModel->index(i, 1));

  QVBoxLayout* sections_layout = new QVBoxLayout();
  sections_layout->addWidget(new QLabel("Header"));
  sections_layout->addWidget(table_view);

  QScrollArea* attributes_container = new QScrollArea();
  attributes_container->setLayout(sections_layout);

  emit NewAttributesArea(attributes_container);

  emit NewStatus(RESULT_SUCCESS);
  return RESULT_SUCCESS;
}

ResultCode BFRESNode::LoadGroupAttributeArea(int groupNum)
{
  m_delegate_group = CustomDelegate::DelegateGroup();

  QStandardItemModel* group_attributes_model = new QStandardItemModel();

  int row = 0;

  group_attributes_model->setItem(row, 0, new QStandardItem("Offset"));
  group_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_offsets[groupNum], 16)));
  row++;

  group_attributes_model->setItem(row, 0, new QStandardItem("Number of files"));
  group_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_counts[groupNum], 16)));
  m_delegate_group.spin_box_delegates.append(1);

  group_attributes_model->setRowCount(row);
  group_attributes_model->setColumnCount(2);

  QTableView* table_view = new QTableView;
  table_view->setModel(group_attributes_model);
  // stretch out table to fit space
  table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_view->verticalHeader()->hide();
  table_view->horizontalHeader()->hide();

  CustomDelegate* customDelegate = new CustomDelegate(m_delegate_group);
  table_view->setItemDelegate(customDelegate);

  // TODO: if this is called more than once, might be a memory leak
  QVBoxLayout* sections_layout = new QVBoxLayout();
  sections_layout->addWidget(new QLabel("Group " + QString::number(groupNum)));
  sections_layout->addWidget(table_view);

  QScrollArea* attributes_container = new QScrollArea();
  attributes_container->setLayout(sections_layout);

  emit NewAttributesArea(attributes_container);
  return RESULT_SUCCESS;
}

QStandardItem* BFRESNode::MakeListItemFromRawList(QVector<BFRES::Node*> list,
                                                  BFRES::GroupType group)
{
  QStandardItem* item = new QStandardItem();
  // Skip the root node
  for (int row = 1; row < list.size(); row++)
  {
    QStandardItem* child_item = new QStandardItem;
    child_item->setText(list[row]->name);
    // TODO: this is hardcoded, do smth else, with the group
    switch (group)
    {
    case BFRES::GROUP_FTEX:
      child_item->setData(NODE_FTEX, Qt::UserRole + 1);
      child_item->setData(row, Qt::UserRole + 2);
      break;
    default:
      child_item->setData(NODE_NONE, Qt::UserRole + 1);
      break;
    }
    item->appendRow(child_item);
  }
  return item;
}

QStandardItem* BFRESNode::MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node)
{
  if (node)
  {
    // TODO: Might have to free this up
    QStandardItem* item = new QStandardItem();
    item->setData(node->name, Qt::DisplayRole);
    if (!m_node_blacklist.contains(node->left_index))
    {
      // TODO: use the better way in BFRES class
      if (blacklist_node != -1)
        m_node_blacklist.append(blacklist_node);

      QStandardItem* left_node_item = MakeTreeItemFromSubtree(node->left_node, node->left_index);
      if (left_node_item)
        item->appendRow(left_node_item);
    }
    if (!m_node_blacklist.contains(node->right_index))
    {
      if (blacklist_node != -1)
        m_node_blacklist.append(blacklist_node);

      QStandardItem* right_node_item = MakeTreeItemFromSubtree(node->right_node, node->right_index);
      if (right_node_item)
        item->appendRow(right_node_item);
    }
    return item;
  }
  else
  {
    return nullptr;
  }
}

void BFRESNode::HandleAttributeItemChange(QStandardItem* item)
{
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;
#ifdef DEBUG
  qDebug() << "Item changed. Row " << item->row() << " column " << item->column();
#endif

  if (m_delegate_group.line_edit_delegates.contains(item->row()))
  {
    if (item->row() == 0)
      m_bfres_header.magic = item->text();
  }
  else if (m_delegate_group.combo_box_delegates.contains(item->row()))
  {
    // TODO: compare to emun instead of hardcoded numbers
    if (item->row() == 1)
    {
      if (item->text() == "Big")
        m_bfres_header.bom = 0xFEFF;
      else if (item->text() == "Little")
        m_bfres_header.bom = 0xFFFE;
    }
  }
  m_bfres.setHeader(m_bfres_header);
  // TODO: spin box reaction
}

void BFRESNode::handleFileTreeClick(QModelIndex index)
{
  if (index.data(Qt::UserRole + 1) == NODE_ROOT_ITEM)
    LoadAttributeArea();

  else if (index.data(Qt::UserRole + 1) == NODE_BFRES_GROUP)
    // todo: generate returns an int error code, do something with it
    LoadGroupAttributeArea(index.data(Qt::UserRole + 2).toInt());

  else if (index.data(Qt::UserRole + 1) == NODE_FTEX)
  {
    FTEX* ftex =
        new FTEX(m_bfres.getFile(),
                 m_bfres.GetRawNodeLists()[1][index.data(Qt::UserRole + 2).toInt()]->data_ptr);
    // temporary, should take a save file dialog eventually
    ftex->SetName(QString("/home/kyle/External/" +
                          m_bfres.GetRawNodeLists()[1][index.data(Qt::UserRole + 2).toInt()]->name +
                          ".dds"));
    // qDebug("FTEX node clicked. Node %i", index.data(Qt::UserRole+2).toInt());
    FTEXNode* ftexNode = new FTEXNode(ftex);

    emit ConnectNode(ftexNode);
    ftexNode->LoadAttributeArea();
    ftexNode->LoadMainWidget();
  }
}
