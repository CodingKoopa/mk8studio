#pragma once

#include "Formats/Models/FMDL.h"
#include "Formats/Models/FVTX.h"
#include "Nodes/Node.h"

class FMDLNode : public Node
{
  Q_OBJECT
public:
  explicit FMDLNode(const FMDL& fmdl, QObject* parent = 0);

  CustomStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  FMDL m_fmdl = FMDL();
  FMDL::Header m_fmdl_header = FMDL::Header();
  QVector<FVTX> m_fvtx_list = QVector<FVTX>();

  bool m_header_loaded = false;
  bool m_fvtxs_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
