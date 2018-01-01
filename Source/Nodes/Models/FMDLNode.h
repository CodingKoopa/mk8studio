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
  FMDL m_fmdl;
  FMDL::Header m_fmdl_header;
  QVector<FVTX> m_fvtx_list;

  bool m_header_loaded = false;
  bool m_fvtxs_loaded = false;

signals:
  void NewFMDL(const FMDL&);

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
