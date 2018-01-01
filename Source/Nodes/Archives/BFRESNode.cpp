#include "BFRESNode.h"

#include <QAction>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItem>
#include <QTableView>

#include "Common.h"
#include "CustomDelegate.h"
#include "CustomStandardItem.h"
#include "ImageView.h"
#include "Nodes/Archives/BFRESGroupNode.h"

BFRESNode::BFRESNode(const BFRES& bfres, QObject* parent) : Node(parent), m_bfres(bfres)
{
}

ResultCode BFRESNode::LoadFileTreeArea()
{
  emit NewStatus(ResultCode::UpdateStatusBar, "Loading file tree...");

  ResultCode res = m_bfres.ReadHeader();
  if (res != ResultCode::Success)
  {
    NewStatus(res);
    return res;
  }
  m_bfres_header = m_bfres.GetHeader();

  QStandardItemModel* file_tree_model = new QStandardItemModel(0, 1);

  QStandardItem* root_item = MakeItem();
  file_tree_model->appendRow(root_item);

  m_tree_view = new QTreeView;
  // stretch out table to fit space
  m_tree_view->header()->hide();
  m_tree_view->setItemDelegate(new CustomItemDelegate(CustomItemDelegate::DelegateGroup()));
  m_tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tree_view, &QTreeView::customContextMenuRequested, this,
          &BFRESNode::HandleTreeCustomContextMenuRequest);

  m_tree_view->setModel(file_tree_model);
  // TODO: this only works with mouse clicking, i still have to figure out
  // keyboard arrow navigation
  connect(m_tree_view, &QTreeView::clicked, this, &BFRESNode::HandleFileTreeClick);

  QVBoxLayout* file_tree_layout = new QVBoxLayout();
  file_tree_layout->addWidget(m_tree_view);

  QScrollArea* file_tree_container = new QScrollArea();
  file_tree_container->setLayout(file_tree_layout);

  emit NewFileTreeArea(file_tree_container);

  emit NewStatus(res);
  return res;
}

CustomStandardItem* BFRESNode::MakeItem()
{
  CustomStandardItem* bfres_item = new CustomStandardItem();
  bfres_item->setData(QString(m_bfres_header.file_name + " (BFRES)"), Qt::DisplayRole);
  // Store the current instance in a QVariant as an upcasted Node pointer.
  // Will be downcasted back to a BFRES Group Node later.
  bfres_item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  m_bfres.ReadDictionaries();

  // Allocate a new Node derived object.
  BFRESGroupNode<FMDL>* fmdl_group_node =
      new BFRESGroupNode<FMDL>(m_bfres.GetFMDLDictionary(), this);
  connect(fmdl_group_node, &BFRESGroupNode<FMDL>::ConnectNode, this, &BFRESNode::ConnectNode);
  emit ConnectNode(fmdl_group_node);
  bfres_item->appendRow(fmdl_group_node->MakeItem());

  BFRESGroupNode<FTEX>* ftex_group_node =
      new BFRESGroupNode<FTEX>(m_bfres.GetFTEXDictionary(), this);
  connect(ftex_group_node, &BFRESGroupNode<FTEX>::ConnectNode, this, &BFRESNode::ConnectNode);
  emit ConnectNode(ftex_group_node);
  bfres_item->appendRow(ftex_group_node->MakeItem());

  return bfres_item;
}

ResultCode BFRESNode::LoadAttributeArea()
{
  emit NewStatus(ResultCode::UpdateStatusBar, "Loading file info...");

  QStandardItemModel* header_attributes_model = new QStandardItemModel();
  m_delegate_group = CustomItemDelegate::DelegateGroup();

  int row = 0;

  // Magic
  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  CustomStandardItem* magic_item = new CustomStandardItem(m_bfres_header.magic);
  magic_item->SetFunction([this](QString value) { m_bfres_header.magic = value; });
  header_attributes_model->setItem(row, 1, magic_item);
  m_delegate_group.line_edit_delegates << row;
  ++row;

  // TODO: Add proper entries for the unknowns, as some are found now.

  // Unknown A
  header_attributes_model->setItem(row, 0, new QStandardItem("Unknown A"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem(QString("0x" + QString::number(m_bfres_header.unknown_a, 16))));
  ++row;

  // Unknown A
  header_attributes_model->setItem(row, 0, new QStandardItem("Unknown B"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem(QString("0x" + QString::number(m_bfres_header.unknown_b, 16))));
  ++row;

  // Unknown A
  header_attributes_model->setItem(row, 0, new QStandardItem("Unknown C"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem(QString("0x" + QString::number(m_bfres_header.unknown_c, 16))));
  ++row;

  // Unknown A
  header_attributes_model->setItem(row, 0, new QStandardItem("Unknown D"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem(QString("0x" + QString::number(m_bfres_header.unknown_d, 16))));
  ++row;

  // Unknown A
  header_attributes_model->setItem(row, 0, new QStandardItem("Unknown E"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem(QString("0x" + QString::number(m_bfres_header.unknown_e, 16))));
  ++row;

  // Unknown A
  header_attributes_model->setItem(row, 0, new QStandardItem("Unknown F"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem(QString("0x" + QString::number(m_bfres_header.unknown_f, 16))));
  ++row;

  // Endianess
  header_attributes_model->setItem(row, 0, new QStandardItem("Endianness"));
  QStandardItemModel* endian_entries = new QStandardItemModel(2, 0);
  QVector<BFRES::EndianInfo> endian_info_list = m_bfres.GetEndianInfoList();
  for (qint32 row = 0; row < endian_info_list.size(); ++row)
    endian_entries->setItem(row, 0, new QStandardItem(endian_info_list[row].name));
  m_delegate_group.combo_box_entries << endian_entries;
  m_delegate_group.combo_box_selections
      << m_bfres.GetEndianIndexFromInfo(m_bfres_header.endian_info);
  m_delegate_group.combo_box_delegates << row;
  CustomStandardItem* endianness_item = new CustomStandardItem(m_bfres_header.endian_info.name);
  endianness_item->SetFunction(
      [this](QString value) { m_bfres_header.endian_info = m_bfres.GetEndianInfoFromName(value); });
  header_attributes_model->setItem(row, 1, endianness_item);
  ++row;

  // Length
  header_attributes_model->setItem(row, 0, new QStandardItem("Length"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem(QString::number(m_bfres_header.length)));
  m_delegate_group.spin_box_delegates << row;
  ++row;

  // Alignment
  header_attributes_model->setItem(row, 0, new QStandardItem("Alignment"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.alignment, 16)));
  ++row;

  // File name offset
  header_attributes_model->setItem(row, 0, new QStandardItem("File Name Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.file_name_offset, 16)));
  // TODO: hex spinbox delegate maybe?
  ++row;

  // String table length
  header_attributes_model->setItem(row, 0, new QStandardItem("String Table Length"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.string_table_length, 16)));
  ++row;

  // String table offset
  header_attributes_model->setItem(row, 0, new QStandardItem("String Table Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(m_bfres_header.string_table_offset, 16)));
  ++row;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  connect(header_attributes_model, &QStandardItemModel::itemChanged, this,
          &BFRESNode::HandleAttributeItemChange);

  // at this point, the model is ready to go, and be put into a view

  QTableView* table_view = new QTableView();

  table_view->setModel(header_attributes_model);
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
  // for (int i = 0; i < sectionHeaderModel->rowCount(); ++i)
  // tableView->openPersistentEditor(sectionHeaderModel->index(i, 1));

  QVBoxLayout* sections_layout = new QVBoxLayout();
  sections_layout->addWidget(new QLabel("Header"));
  sections_layout->addWidget(table_view);

  QScrollArea* attributes_container = new QScrollArea();
  attributes_container->setLayout(sections_layout);

  emit NewAttributesArea(attributes_container);

  emit NewStatus(ResultCode::Success);
  return ResultCode::Success;
}

void BFRESNode::HandleAttributeItemChange(QStandardItem* item)
{
  // Colum 1 is the only editable row
  if (item->column() != 1)
    return;
#ifdef DEBUG
  qDebug() << "Item changed. Row " << item->row() << " column " << item->column();
#endif

  CustomStandardItem* custom_item = dynamic_cast<CustomStandardItem*>(item);
  if (custom_item)
    custom_item->ExecuteFunction();

  m_bfres.SetHeader(m_bfres_header);
}