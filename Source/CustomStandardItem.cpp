#include "CustomStandardItem.h"

#include "Nodes/Node.h"

CustomStandardItem::CustomStandardItem(QString string) : QStandardItem(string) {}

CustomStandardItem::~CustomStandardItem()
{
  // If a Node object has been attatched to this item.
  if (Node* node = qvariant_cast<Node*>(data(Qt::UserRole + 1)))
    delete node;
}

void CustomStandardItem::ExecuteFunction()
{
  if (m_function_type == FunctionType::Index && m_index_function)
    m_index_function(index().row());
  else if (m_function_type == FunctionType::Value && m_value_function)
    m_value_function(text());
}

void CustomStandardItem::SetFunction(const std::function<void(quint32)>& function)
{
  m_index_function = function;
  m_function_type = FunctionType::Index;
}

void CustomStandardItem::SetFunction(const std::function<void(QString)>& function)
{
  m_value_function = function;
  m_function_type = FunctionType::Value;
}
