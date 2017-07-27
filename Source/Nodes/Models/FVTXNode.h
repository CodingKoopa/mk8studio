#pragma once

#include "Formats/Models/FVTX.h"
#include "Nodes/Node.h"

class FVTXNode : public Node
{
  Q_OBJECT
public:
  explicit FVTXNode(const FVTX& fvtx, QObject* parent = 0);

  CustomStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  FVTX m_fvtx = FVTX();
  FVTX::Header m_fvtx_header = FVTX::Header();

  bool m_header_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
