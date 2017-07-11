#include "CustomStandardItem.h"

#include "Nodes/Node.h"

CustomStandardItem::CustomStandardItem(QString string) : QStandardItem(string)
{
}

CustomStandardItem::~CustomStandardItem()
{
  // If a Node object has been attatched to this item.
  if (Node* node = qvariant_cast<Node*>(data(Qt::UserRole + 1)))
    delete node;
}

void CustomStandardItem::ExecuteFunction()
{
  if (m_function)
    m_function(text());
}

void CustomStandardItem::SetFunction(const std::function<void(QString)>& value)
{
  m_function = value;
}
