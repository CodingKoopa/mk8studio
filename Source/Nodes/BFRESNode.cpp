#include "BFRESNode.h"

#include <QAction>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItem>
#include <QTableView>

#include "BFRESGroupNode.h"
#include "Common.h"
#include "CustomDelegate.h"
#include "ExportDialog.h"
#include "ImageView.h"

BFRESNode::BFRESNode(BFRES* bfres, QObject* parent) : Node(parent), m_bfres(bfres)
{
}

ResultCode BFRESNode::LoadFileTreeArea()
{
  // This doesn't seem like the right place to put this, but the ctor doesn't work
  emit ConnectNode(this);

  emit NewStatus(RESULT_STATUS_BAR_UPDATE, "Loading file tree...");

  ResultCode res = m_bfres->ReadHeader();
  if (res != RESULT_SUCCESS)
  {
    NewStatus(res);
    return res;
  }
  m_bfres_header = m_bfres->GetHeader();
  // TODO: have this return a result code
  m_bfres->ReadIndexGroups();
  // QVector<BFRES::Node*> root_nodes = bfres.GetRootNodes();
  QVector<QVector<BFRES::Node*>> raw_node_list = m_bfres->GetRawNodeLists();

  QStandardItemModel* file_tree_model = new QStandardItemModel(0, 1);

  QStandardItem* root_item = MakeItem();
  file_tree_model->appendRow(root_item);

  m_tree_view = new QTreeView;
  // stretch out table to fit space
  m_tree_view->header()->hide();
  m_tree_view->setItemDelegate(new CustomDelegate(CustomDelegate::DelegateGroup()));
  m_tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tree_view, SIGNAL(customContextMenuRequested(const QPoint&)), this,
          SLOT(HandleTreeCustomContextMenuRequest(const QPoint&)));

  m_tree_view->setModel(file_tree_model);
  // todo: this only works with mouse clicking, i still have to figure out
  // keyboard arrow navigation
  connect(m_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(HandleFileTreeClick(QModelIndex)));
  connect(m_tree_view, SIGNAL(doubleClicked(QModelIndex)), this,
          SLOT(HandleFileTreeClick(QModelIndex)));
  m_file_tree_container = new QScrollArea();

  m_file_tree_layout = new QVBoxLayout();
  m_file_tree_layout->addWidget(m_tree_view);

  m_file_tree_container = new QScrollArea();
  m_file_tree_container->setLayout(m_file_tree_layout);

  emit NewFileTreeArea(m_file_tree_container);

  emit NewStatus(res);
  return res;
}

QStandardItem* BFRESNode::MakeItem()
{
  QStandardItem* bfres_item = new QStandardItem();
  bfres_item->setData(QString(m_bfres_header.file_name + " (BFRES)"), Qt::DisplayRole);
  // Store the current instance in a QVariant as an upcasted Node pointer.
  // Will be downcasted back to a BFRES Group Node later.
  bfres_item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  for (int group = 0; group < m_bfres_header.file_counts.size(); group++)
  {
    if (m_bfres_header.file_counts[group])
    {
      // Allocate a new Node derived object.
      BFRESGroupNode* group_node =
          new BFRESGroupNode(group, m_bfres, m_bfres->GetRawNodeLists()[group], this);
      connect(group_node, SIGNAL(ConnectNode(Node*)), this, SIGNAL(ConnectNode(Node*)));
      emit ConnectNode(group_node);
      bfres_item->appendRow(group_node->MakeItem());
    }
  }
  return bfres_item;
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
  header_attributes_model->setItem(row, 0, new QStandardItem("Endianness"));
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
  m_bfres->setHeader(m_bfres_header);
  // TODO: spin box reaction
}

void BFRESNode::HandleTreeCustomContextMenuRequest(const QPoint& point)
{
  qDebug() << "It's dude.";
  current_index = m_tree_view->indexAt(point);

  QMenu context_menu;

  //  switch (current_index.data(Qt::UserRole + 2).toInt())
  //  {
  //  case NODE_FTEX:
  //  {
  //    QAction* action_export = new QAction("Export");
  //    connect(action_export, SIGNAL(triggered()), this, SLOT(HandleExport()));
  //    context_menu.addAction(action_export);
  //    context_menu.exec(m_tree_view->mapToGlobal(point));
  //    break;
  //  }
  //  default:
  //    break;
  //  }
}

void BFRESNode::HandleFileTreeClick(QModelIndex index)
{
  if (BFRESNode* bfres_node = qvariant_cast<BFRESNode*>(index.data(Qt::UserRole + 1)))
    bfres_node->LoadAttributeArea();
  else if (FTEXNode* ftex_node = qvariant_cast<FTEXNode*>(index.data(Qt::UserRole + 1)))
  {
    ftex_node->LoadAttributeArea();
    ftex_node->LoadMainWidget();
  }

  //  case NODE_BFRES_GROUP:
  //    // todo: generate returns an int error code, do something with it
  //    LoadGroupAttributeArea(index.data(Qt::UserRole + 2).toInt());
  //    break;

  //  case NODE_FTEX:
  //    FTEX* ftex =
  //        new FTEX(m_bfres->getFile(),
  //                 m_bfres->GetRawNodeLists()[1][index.data(Qt::UserRole + 2).toInt()]->data_ptr);
  //    // temporary, should take a save file dialog eventually
  //    ftex->SetName(QString("/home/kyle/External/" +
  //                          m_bfres->GetRawNodeLists()[1][index.data(Qt::UserRole +
  //                          2).toInt()]->name +
  //                          ".dds"));
  //    // qDebug("FTEX node clicked. Node %i", index.data(Qt::UserRole+2).toInt());
  //    FTEXNode* ftexNode = new FTEXNode(ftex);

  //    emit ConnectNode(ftexNode);
  //    ftexNode->LoadAttributeArea();
  //    ftexNode->LoadMainWidget();
  //    break;
  //  }
}

void BFRESNode::HandleExport()
{
  qDebug() << "yo dude";
  ExportDialog export_dialog;
  export_dialog.exec();
}
