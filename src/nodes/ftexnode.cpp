#include <QDebug>
#include <QHeaderView>
#include <QLabel>

#include "customdelegate.h"

#include "ftexnode.h"

FTEXNode::FTEXNode(FTEX* ftex, QObject* parent) : Node(parent), m_ftex(ftex)
{
  // TODO: empty / one liners should maybe just be moved to headers?
}

ResultCode FTEXNode::LoadAttributeArea()
{
  QStandardItemModel* sectionHeaderModel = new QStandardItemModel(0, 2);
  // TODO: get result code from this
  m_ftex->ReadHeader();
  header = m_ftex->GetHeader();
  int row = 0;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem(header.magic));
  m_delegate_group.line_edit_delegates << 0;
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Header Offset"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem(QString::number(m_ftex->GetStart(), 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Texture Offset"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.data_offset, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Texture Length"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.data_length, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Mipmap Texture Offset"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.mipmapOffset, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Mipmap Size"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.mipSize, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Number of Mipmaps"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.num_mips, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Width"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem(QString::number(header.width)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Height"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem(QString::number(header.height)));
  row++;

  // TODO: make this a combobox with all available values
  sectionHeaderModel->setItem(row, 0, new QStandardItem("Format"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem("0x" + QString::number(header.format, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Usage"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem("0x" + QString::number(header.usage, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Tiling"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.tile_mode, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("AA Mode"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem("0x" + QString::number(header.aaMode, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Swizzle"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.swizzle, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Depth"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem("0x" + QString::number(header.depth, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Dim"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem("0x" + QString::number(header.dim, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Pitch"));
  sectionHeaderModel->setItem(row, 1, new QStandardItem("0x" + QString::number(header.pitch, 16)));
  row++;

  sectionHeaderModel->setItem(row, 0, new QStandardItem("Alignment"));
  sectionHeaderModel->setItem(row, 1,
                              new QStandardItem("0x" + QString::number(header.alignment, 16)));
  row++;

  sectionHeaderModel->setRowCount(row);

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

  QVBoxLayout* sectionsLay = new QVBoxLayout();
  sectionsLay->addWidget(new QLabel("Header"));
  sectionsLay->addWidget(tableView);

  QScrollArea* sectionsContainer = new QScrollArea();
  sectionsContainer->setLayout(sectionsLay);

  emit NewAttributesArea(sectionsContainer);
  return RESULT_SUCCESS;
}

ResultCode FTEXNode::LoadMainWidget()
{
  ResultCode res = m_ftex->ReadImageData();
  if (res != RESULT_SUCCESS)
  {
    emit NewStatus(res);
    return res;
  }
  ImageView* imageView = new ImageView(m_ftex->GetImage());
  emit NewMainWidget(imageView);
  emit NewStatus(res);
  return RESULT_SUCCESS;
}

void FTEXNode::HandleAttributeItemChange(QStandardItem* item)
{
  // this is where changes made in the table would be handled
  // see: BFRESGUI::handleHeaderItemChange
  qDebug() << item->data().toString();
}
