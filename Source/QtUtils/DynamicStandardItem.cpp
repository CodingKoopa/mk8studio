#include "DynamicStandardItem.h"

#include "Nodes/Node.h"

DynamicStandardItem::DynamicStandardItem(QString string) : QStandardItem(string) {}

DynamicStandardItem::~DynamicStandardItem()
{
  // If a Node object has been attatched to this item.
  if (Node* node = qvariant_cast<Node*>(data(Qt::UserRole + 1)))
    delete node;
}

void DynamicStandardItem::ExecuteFunction()
{
  if (m_function_type == FunctionType::Index && m_index_function)
    m_index_function(index().row());
  else if (m_function_type == FunctionType::Value && m_value_function)
    m_value_function(text());
}

void DynamicStandardItem::SetFunction(const std::function<void(quint32)>& function)
{
  m_index_function = function;
  m_function_type = FunctionType::Index;
}

void DynamicStandardItem::SetFunction(const std::function<void(QString)>& function)
{
  m_value_function = function;
  m_function_type = FunctionType::Value;
}
