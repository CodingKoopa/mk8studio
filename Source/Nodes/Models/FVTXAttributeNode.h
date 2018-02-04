#pragma once

#include "Formats/Models/FVTX.h"
#include "Nodes/Node.h"

class FVTXAttributeNode : public Node
{
  Q_OBJECT
public:
  explicit FVTXAttributeNode(const FVTX::Attribute& attribute, QString attribute_format_name,
                             QString attribute_friendly_name, QObject* parent = 0);

  CustomStandardItem* MakeItem() override;
  ResultCode LoadAttributeArea() override;

private:
  FVTX::Attribute m_attribute;
  QString m_attribute_format_name;
  QString m_attribute_friendly_name;

private slots:
  void HandleAttributeItemChange(QStandardItem* item) override;
};
