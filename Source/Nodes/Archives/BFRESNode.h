#pragma once

#include <QScrollArea>
#include <QStandardItem>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "CustomDelegate.h"
#include "CustomStandardItem.h"
#include "Formats/Archives/BFRES.h"
#include "Nodes/Textures/FTEXNode.h"

class BFRESNode : public Node
{
  Q_OBJECT
public:
  explicit BFRESNode(const BFRES& bfres, QObject* parent = 0);
  ResultCode LoadFileTreeArea() override;
  CustomStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  BFRES m_bfres;
  BFRES::Header m_bfres_header;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};

// Make this class known to the template-based functions to allow casting to and from a QVariant.
Q_DECLARE_METATYPE(BFRESNode*)
