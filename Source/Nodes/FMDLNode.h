#ifndef FMDLNODE_H
#define FMDLNODE_H

#include "Formats/FMDL.h"
#include "Node.h"

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

  bool m_header_loaded = false;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};

#endif  // FMDLNODE_H
