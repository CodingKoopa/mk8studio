#pragma once

#include "Formats/Models/FVTX.h"
#include "Nodes/Node.h"

class FVTXBufferNode : public Node
{
  Q_OBJECT
public:
  explicit FVTXBufferNode(const FVTX::Buffer& buffer, QObject* parent = 0);

  DynamicStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  FVTX::Buffer m_buffer;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
