#ifndef FTEXGUI_H
#define FTEXGUI_H

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
  explicit FTEXNode(FTEX* ftex, QObject* parent = 0)
      : Node(parent), m_ftex(ftex), m_header_loaded(false), m_image_loaded(false)
  {
  }
  ~FTEXNode() { delete m_ftex; }
  QStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;
  ResultCode LoadMainWidget() override;

private:
  FTEX* m_ftex;
  FTEX::FTEXHeader m_header;

  QTableView* m_table_view;
  QList<QGroupBox*> m_groups_list;
  QLineEdit* m_path_line_edit;

  bool m_header_loaded;
  bool m_image_loaded;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
  void HandleExportActionClick();
  void HandleExport();
};

#endif  // FTEXGUI_H
