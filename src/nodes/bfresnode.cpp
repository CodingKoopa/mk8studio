#include <QDebug>
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
  rootItem->setData(ROOT_ITEM, Qt::UserRole + 1);
  for (int group = 0; group < m_bfres_header.file_counts.size(); group++)
  {
    if (m_bfres_header.file_counts[group] != 0)
    {
      // For BST testing, use this:
      //      if (bfres.GetRootNodes()[group])
      //      {
      //        QStandardItem* group_item = MakeTreeItemFromSubtree(bfres.GetRootNodes()[group]);
      //        m_node_blacklist.clear();
      // (/BST stuff)
      QStandardItem* group_item = MakeListItemFromRawList(raw_node_list[group], group);
      if (group_item)
      {
        // the string to be displayed
        group_item->setData("Group " + QString::number(group), Qt::DisplayRole);
        // the type of item
        group_item->setData(BFRES_GROUP, Qt::UserRole + 1);
        // the data itself
        group_item->setData(group, Qt::UserRole + 2);

        rootItem->appendRow(group_item);
      }
      //}
    }
  }
  fileTreeModel->appendRow(rootItem);

  // TODO: this might not have to be a member var?
  treeView = new QTreeView;
  // there might be a right click menu delegate here in the (probably far)
  // future
  CustomDelegate* customDelegate = new CustomDelegate(CustomDelegate::delegateGroup_t());
  treeView->setItemDelegate(customDelegate);
  treeView->setModel(fileTreeModel);
  // todo: this only works with mouse clicking, i still have to figure out
  // keyboard arrow navigation
  connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(handleFileTreeClick(QModelIndex)));
  connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this,
          SLOT(handleFileTreeClick(QModelIndex)));
  m_file_tree_container = new QScrollArea();

  m_file_tree_layout = new QVBoxLayout();
  m_file_tree_layout->addWidget(treeView);

  m_file_tree_container = new QScrollArea();
  m_file_tree_container->setLayout(m_file_tree_layout);

  emit NewFileTreeArea(m_file_tree_container);

  emit NewStatus(res);
  return res;
}

ResultCode BFRESNode::LoadAttributeArea()
{
  emit NewStatus(RESULT_STATUS_BAR_UPDATE, "Loading file info...");

  QStandardItemModel* sectionHeaderModel = new QStandardItemModel(7, 2);

  int row = 0;

  // Magic
  sectionHeaderModel->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  QStandardItem* magicValueItem = new QStandardItem(m_bfres_header.magic);
  sectionHeaderModel->setItem(row, 1, magicValueItem);
  m_delegate_group.line_edit_delegates << 0;
  row++;

  // Unknown A
  // this probably isn't really necessary to show
  //  sectionHeaderModel->setItem(row, 0, new QStandardItem("Unknown 1"));
  //  QStandardItem* magicValueItem = new QStandardItem(m_bfres_header.unknown_a);
  //  sectionHeaderModel->setItem(row, 1, magicValueItem);
  //  m_delegate_group.line_edit_delegates << 0;
  //  row++;

  // Endianess
  sectionHeaderModel->setItem(row, 0, new QStandardItem("Endianess"));
  QStandardItemModel* endianEntries = new QStandardItemModel(2, 0);
  endianEntries->setItem(0, 0, new QStandardItem("Big"));
  endianEntries->setItem(1, 0, new QStandardItem("Little"));
  m_delegate_group.combo_box_entries << endianEntries;
  m_delegate_group.combo_box_delegates << 1;

  QStandardItem* endianTableItem = new QStandardItem();

  if (m_bfres_header.bom == 0xFEFF)
  {
    // Visually set text
    endianTableItem->setText("Big");
    // Internally set index
    m_delegate_group.combo_box_selections << 0;
  }
  else if (m_bfres_header.bom == 0xFFFE)
  {
    endianTableItem->setText("Little");
    m_delegate_group.combo_box_selections << 1;
  }
  else
  {
    NewStatus(RESULT_BFRES_ENDIANNESS);
    return RESULT_SUCCESS;
  }
  sectionHeaderModel->setItem(row, 1, endianTableItem);
  row++;

  // Length
  sectionHeaderModel->setItem(row, 0, new QStandardItem("Length"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem(QString::number(m_bfres_header.length)));
  m_delegate_group.spin_box_delegates << 2;
  row++;

  // Alignment
  sectionHeaderModel->setItem(row, 0, new QStandardItem("Alignment"));
  sectionHeaderModel->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.alignment, 16)));
  row++;

  // File name offset
  sectionHeaderModel->setItem(row, 0, new QStandardItem("File Name Offset"));
  sectionHeaderModel->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_name_offset, 16)));
  // TODO: hex spinbox delegate maybe?
  row++;

  // String table length
  sectionHeaderModel->setItem(row, 0, new QStandardItem("String Table Length"));
  sectionHeaderModel->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.string_table_length, 16)));
  row++;

  // String table offset
  sectionHeaderModel->setItem(row, 0, new QStandardItem("String Table Offset"));
  sectionHeaderModel->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.string_table_offset, 16)));
  row++;

  // Row 7+: File group offsets
  for (int i = 0; i < m_bfres_header.file_offsets.size(); i++)
  {
    if (m_bfres_header.file_offsets[i] != 0)
    {
      sectionHeaderModel->setItem(row, 0,
                                  new QStandardItem("Group " + QString::number(i) + " Offset"));
      sectionHeaderModel->setItem(
          row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_offsets[i], 16)));
      row += 2;
    }
  }
  row++;

  // Row 8+: File group count
  for (int i = 0; i < m_bfres_header.file_counts.size(); i++)
  {
    if (m_bfres_header.file_counts[i] != 0)
    {
      sectionHeaderModel->setItem(
          row, 0, new QStandardItem("Number of files in group " + QString::number(i)));
      sectionHeaderModel->setItem(
          row, 1, new QStandardItem(QString::number(m_bfres_header.file_counts[i])));
      m_delegate_group.spin_box_delegates << row;
      row += 2;
    }
  }

  QObject::connect(sectionHeaderModel, SIGNAL(itemChanged(QStandardItem*)), this,
                   SLOT(HandleAttributeItemChange(QStandardItem*)));

  // at this point, the model is ready to go, and be put into a view

  tableView = new QTableView;

  tableView->setModel(sectionHeaderModel);
  // stretch out table to fit space
  tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  tableView->verticalHeader()->hide();
  tableView->horizontalHeader()->hide();

  tableView->setItemDelegate(new CustomDelegate(m_delegate_group));

  // To have all editors open by default, uncomment this out
  // PROS: Looks nicer, possibly more convienient
  // CONS: Scrolling can accidentally change values, and opening a new sections
  // seems to select the text
  // in every open editor for some reason
  // for (int i = 0; i < sectionHeaderModel->rowCount(); i++)
  // tableView->openPersistentEditor(sectionHeaderModel->index(i, 1));

  QVBoxLayout* sections_layout = new QVBoxLayout();
  sections_layout->addWidget(new QLabel("Header"));
  sections_layout->addWidget(tableView);

  QScrollArea* attributes_container = new QScrollArea();
  attributes_container->setLayout(sections_layout);

  emit NewAttributesArea(attributes_container);

  emit NewStatus(RESULT_SUCCESS);
  return RESULT_SUCCESS;
}

ResultCode BFRESNode::LoadGroupAttributeArea(int groupNum)
{
  QStandardItemModel* sectionGroupModel = new QStandardItemModel(7, 2);

  sectionGroupModel->setItem(0, 0,
                             new QStandardItem("Group " + QString::number(groupNum) + " Offset"));

  QStandardItem* stringTblOffItem =
      new QStandardItem("0x" + QString::number(m_bfres_header.file_offsets[groupNum], 16));
  sectionGroupModel->setItem(0, 1, stringTblOffItem);

  // TODO: add number of files

  tableView = new QTableView;
  tableView->setModel(sectionGroupModel);
  // stretch out table to fit space
  tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  tableView->verticalHeader()->hide();
  tableView->horizontalHeader()->hide();

  CustomDelegate* customDelegate = new CustomDelegate(m_delegate_group);
  tableView->setItemDelegate(customDelegate);

  // TODO: if this is called more than once, might be a memory leak
  QVBoxLayout* sections_layout = new QVBoxLayout();
  sections_layout->addWidget(new QLabel("lol"));
  sections_layout->addWidget(tableView);

  QScrollArea* attributes_container = new QScrollArea();
  attributes_container->setLayout(sections_layout);

  emit NewAttributesArea(attributes_container);
  return RESULT_SUCCESS;
}

ResultCode BFRESNode::SaveFile()
{
  emit NewStatus(RESULT_STATUS_BAR_UPDATE, "Saving BFRES file...");
  ResultCode ret = m_bfres.WriteHeader();
  emit NewStatus(ret);
  return ret;
}

QStandardItem* BFRESNode::MakeListItemFromRawList(QVector<BFRES::Node*> list, int group)
{
  QStandardItem* item = new QStandardItem();
  // Skip the root node
  for (int row = 1; row < list.size(); row++)
  {
    QStandardItem* child_item = new QStandardItem;
    child_item->setText(list[row]->name);
    // TODO: this is hardcoded, do smth else, with the group
    child_item->setData(FTEX_NODE, Qt::UserRole + 1);
    child_item->setData(row, Qt::UserRole + 2);
    item->appendRow(child_item);
  }
  return item;
}

QStandardItem* BFRESNode::MakeTreeItemFromSubtree(BFRES::Node* node, int blacklist_node)
{
  if (node)
  {
    // [!] TODO: Free this up
    QStandardItem* item = new QStandardItem();
    item->setData(node->name, Qt::DisplayRole);
    if (!m_node_blacklist.contains(node->left_index))
    {
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
  if (index.data(Qt::UserRole + 1) == ROOT_ITEM)
    LoadAttributeArea();

  else if (index.data(Qt::UserRole + 1) == BFRES_GROUP)
    // todo: generate returns an int error code, do something with it
    LoadGroupAttributeArea(index.data(Qt::UserRole + 2).toInt());

  else if (index.data(Qt::UserRole + 1) == FTEX_NODE)
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
