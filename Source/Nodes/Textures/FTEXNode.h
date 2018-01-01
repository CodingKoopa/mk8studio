#pragma once

#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTableView>

#include "CustomDelegate.h"
#include "CustomStandardItem.h"
#include "Formats/Textures/FTEX.h"
#include "ImageView.h"
#include "Nodes/Node.h"

class FTEXNode : public Node
{
  Q_OBJECT
public:
  explicit FTEXNode(const FTEX& ftex, QObject* parent = 0);
  CustomStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;
  ResultCode LoadMainWidget() override;

private:
  FTEX m_ftex;
  FTEX::Header m_ftex_header;

  // Unlike the other section containers for other nodes, this one is reused for the injection
  // dialog, so it's a member variable.
  QScrollArea* m_sections_container;

  QTableView* m_table_view;

  QLineEdit* m_path_line_edit;
  QComboBox* m_format_combo_box;

  bool m_header_loaded = false;
  bool m_image_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
  void HandleExportActionClick();
  void HandleExportButtonClick();
  void HandleInjectActionClick();
  void HandleInjectButtonClick();
};
