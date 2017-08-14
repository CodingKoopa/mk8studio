#pragma once

#include "Formats/Models/FVTX.h"
#include "Nodes/Node.h"

class FVTXAttributeNode : public Node
{
  Q_OBJECT
public:
  explicit FVTXAttributeNode(const FVTX::Attribute& attribute, QObject* parent = 0);

  CustomStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  FVTX::Attribute m_attribute;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
