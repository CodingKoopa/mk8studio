#ifndef FTEXGUI_H
#define FTEXGUI_H

#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTableView>

#include "CustomDelegate.h"
#include "Formats/FTEX.h"
#include "ImageView.h"
#include "Node.h"

class FTEXNode : public Node
{
  Q_OBJECT
public:
  explicit FTEXNode(FTEX* ftex, QObject* parent = 0);
  ~FTEXNode() { delete m_ftex; }
  QStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;
  ResultCode LoadMainWidget() override;

private:
  FTEX* m_ftex;
  FTEX::FTEXHeader* m_ftex_header;

  // Unlike the other section containers for other nodes, this one is reused for the injection
  // dialog, so it's a member variable.
  QScrollArea* m_sections_container = nullptr;

  QTableView* m_table_view;

  QLineEdit* m_path_line_edit;
  QComboBox* m_format_combo_box;

  bool m_header_loaded;
  bool m_image_loaded;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
  void HandleExportActionClick();
  void HandleExportButtonClick();
  void HandleInjectActionClick();
  void HandleInjectButtonClick();
};

#endif  // FTEXGUI_H
