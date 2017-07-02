#pragma once

#include <QScrollArea>
#include <QStandardItem>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "CustomDelegate.h"
#include "FTEXNode.h"
#include "Formats/BFRES.h"

class BFRESNode : public Node
{
  Q_OBJECT
public:
  explicit BFRESNode(const BFRES& bfres, QObject* parent = 0);
  ResultCode LoadFileTreeArea() override;
  QStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  BFRES m_bfres;
  BFRES::Header m_bfres_header;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};

// Make this class known to the template-based functions to allow casting to and from a QVariant.
Q_DECLARE_METATYPE(BFRESNode*)
