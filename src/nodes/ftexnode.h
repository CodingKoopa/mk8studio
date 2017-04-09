#ifndef FTEXGUI_H
#define FTEXGUI_H

#include <QScrollArea>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

#include "customdelegate.h"
#include "formats/ftex.h"
#include "imageview.h"
#include "nodegroup.h"

class FTEXNode : public Node
{
  Q_OBJECT
public:
  explicit FTEXNode(FTEX* m_ftex, QObject* parent = 0);
  ~FTEXNode();
  ResultCode LoadAttributeArea() override;
  ResultCode LoadMainWidget() override;

private:
  FTEX* m_ftex;
  FTEX::FTEXHeader header;

  CustomDelegate::DelegateGroup m_delegate_group;
  QTableView* m_table_view;

private slots:
  void HandleAttributeItemChange(QStandardItem* item);
};

#endif  // FTEXGUI_H
