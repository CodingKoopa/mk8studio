#pragma once

#include <memory>

#include "Formats/Models/FVTX.h"
#include "Nodes/Node.h"

class FVTXNode : public Node
{
  Q_OBJECT
public:
  explicit FVTXNode(std::shared_ptr<FVTX> fvtx, QObject* parent = 0);

  DynamicStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  std::shared_ptr<FVTX> m_fvtx;
  FVTX::Header m_fvtx_header;
  QVector<FVTX::Attribute> m_attribute_list;
  QVector<FVTX::Buffer> m_buffer_list;

  bool m_header_loaded = false;
  bool m_attributes_loaded = false;
  bool m_buffers_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
