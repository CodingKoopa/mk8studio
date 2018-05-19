#pragma once

#include <memory>

#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTableView>

#include "Formats/Textures/FTEX.h"
#include "Nodes/Node.h"
#include "QtUtils/DynamicItemDelegate.h"
#include "QtUtils/DynamicStandardItem.h"

class FTEXNode : public Node
{
  Q_OBJECT
public:
  explicit FTEXNode(std::shared_ptr<FTEX> ftex, QObject* parent = 0);
  DynamicStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;
  ResultCode LoadMainWidget() override;

private:
  static const inline QString m_dds_item_string = "DDS";

  static const inline QString m_last_export_path_key = "last_ftex_export_path";
  static const inline QString m_last_inject_path_key = "last_ftex_inject_path";

  std::shared_ptr<FTEX> m_ftex;
  FTEX::Header m_ftex_header;

  // Unlike the other section containers for other nodes, this one is reused for the injection
  // dialog, so it's a member variable.
  QScrollArea* m_sections_container = nullptr;

  QTableView* m_table_view = nullptr;

  QComboBox* m_format_combo_box = nullptr;

  bool m_header_loaded = false;
  bool m_image_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
  void HandleExportAction();
  void HandleExportButton(const QString& path);
  void HandleInjectAction();
  void HandleInjectButton(const QString& path);
};
