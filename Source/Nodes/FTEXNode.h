#ifndef FTEXGUI_H
#define FTEXGUI_H

#include <QScrollArea>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

#include "CustomDelegate.h"
#include "Formats/FTEX.h"
#include "ImageView.h"
#include "Node.h"

class FTEXNode : public Node
{
  Q_OBJECT
public:
  explicit FTEXNode(FTEX* ftex, QObject* parent = 0) : Node(parent), m_ftex(ftex) {}
  ~FTEXNode() { delete m_ftex; }
  QStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;
  ResultCode LoadMainWidget() override;

private:
  FTEX* m_ftex;
  FTEX::FTEXHeader header;

  QTableView* m_table_view;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};

#endif  // FTEXGUI_H
