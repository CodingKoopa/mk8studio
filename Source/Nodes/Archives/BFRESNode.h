#pragma once

#include <memory>

#include <QScrollArea>
#include <QStandardItem>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "Formats/Archives/BFRES.h"
#include "Nodes/Textures/FTEXNode.h"
#include "QtUtils/DynamicItemDelegate.h"
#include "QtUtils/DynamicStandardItem.h"

class BFRESNode : public Node
{
  Q_OBJECT
public:
  explicit BFRESNode(std::shared_ptr<BFRES> bfres, QObject* parent = 0);
  ResultCode LoadFileTreeArea() override;
  DynamicStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  std::shared_ptr<BFRES> m_bfres;
  BFRES::Header m_bfres_header = BFRES::Header();

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
