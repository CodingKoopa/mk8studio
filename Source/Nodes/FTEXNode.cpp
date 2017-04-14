#include "FTEXNode.h"

#include <QHeaderView>
#include <QLabel>

QStandardItem* FTEXNode::MakeItem()
{
  QStandardItem* item = new QStandardItem();
  item->setData(QString(m_ftex->GetName() + " (FTEX)"), Qt::DisplayRole);
  item->setData(QVariant::fromValue<Node*>(static_cast<Node*>(this)), Qt::UserRole + 1);
  return item;
}

ResultCode FTEXNode::LoadAttributeArea()
{
  QStandardItemModel* header_attributes_model = new QStandardItemModel();
  // TODO: get result code from this
  m_ftex->ReadHeader();
  header = m_ftex->GetHeader();
  int row = 0;

  header_attributes_model->setItem(row, 0, new QStandardItem("Magic File Identifier"));
  header_attributes_model->setItem(row, 1, new QStandardItem(header.magic));
  m_delegate_group.line_edit_delegates << 0;
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Header Offset"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem(QString::number(m_ftex->GetStart(), 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(header.data_offset, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Texture Length"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(header.data_length, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Texture Offset"));
  header_attributes_model->setItem(
      row, 1, new QStandardItem("0x" + QString::number(header.mipmap_offset, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Mipmap Size"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.mipSize, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Number of Mipmaps"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.num_mips, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Width"));
  header_attributes_model->setItem(row, 1, new QStandardItem(QString::number(header.width)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Height"));
  header_attributes_model->setItem(row, 1, new QStandardItem(QString::number(header.height)));
  row++;

  // TODO: make this a combobox with all available values
  header_attributes_model->setItem(row, 0, new QStandardItem("Format"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.format, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Usage"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.usage, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Tiling"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.tile_mode, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("AA Mode"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.aa_mode, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Swizzle"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.swizzle, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Depth"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.depth, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Dim"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.dim, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Pitch"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.pitch, 16)));
  row++;

  header_attributes_model->setItem(row, 0, new QStandardItem("Alignment"));
  header_attributes_model->setItem(row, 1,
                                   new QStandardItem("0x" + QString::number(header.alignment, 16)));
  row++;

  header_attributes_model->setRowCount(row);
  header_attributes_model->setColumnCount(2);

  QObject::connect(header_attributes_model, SIGNAL(itemChanged(QStandardItem*)), this,
                   SLOT(HandleAttributeItemChange(QStandardItem*)));

  m_table_view = new QTableView;

  m_table_view->setModel(header_attributes_model);
  // stretch out table to fit space
  m_table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table_view->verticalHeader()->hide();
  m_table_view->horizontalHeader()->hide();

  m_table_view->setItemDelegate(new CustomDelegate(m_delegate_group));

  // To have all editors open by default, uncomment this out
  // PROS: Looks nicer, possibly more convienient
  // CONS: Scrolling can accidentally change values, and opening a new sections
  // seems to select the text
  // in every open editor for some reason
  // for (int i = 0; i < sectionHeaderModel->rowCount(); i++)
  // tableView->openPersistentEditor(sectionHeaderModel->index(i, 1));

  QVBoxLayout* attributes_layout = new QVBoxLayout();
  attributes_layout->addWidget(new QLabel("Header"));
  attributes_layout->addWidget(m_table_view);

  QScrollArea* sectionsContainer = new QScrollArea();
  sectionsContainer->setLayout(attributes_layout);

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
  ImageView* image_view = new ImageView(m_ftex->GetImage());
  emit NewMainWidget(image_view);
  emit NewStatus(res);
  return RESULT_SUCCESS;
}

void FTEXNode::HandleAttributeItemChange(QStandardItem* item)
{
  // this is where changes made in the table would be handled
  // see: BFRESGUI::handleHeaderItemChange
  // (temp obviously)
  item = item;
}
