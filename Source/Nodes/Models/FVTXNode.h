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
  QVector<FVTX::Attribute> m_attribute_list = QVector<FVTX::Attribute>();

  bool m_header_loaded = false;
  bool m_attributes_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
