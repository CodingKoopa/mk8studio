#pragma once

#include <memory>

#include "Formats/Models/FMDL.h"
#include "Formats/Models/FVTX.h"
#include "Nodes/Node.h"

class FMDLNode : public Node
{
  Q_OBJECT
public:
  explicit FMDLNode(std::shared_ptr<FMDL> fmdl, QObject* parent = 0);

  DynamicStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  std::shared_ptr<FMDL> m_fmdl;
  FMDL::Header m_fmdl_header;
  FMDL::FVTXList m_fvtx_list;

  bool m_header_loaded = false;
  bool m_fvtxs_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
