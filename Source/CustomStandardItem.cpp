#include "CustomStandardItem.h"

CustomStandardItem::CustomStandardItem(QString string) : QStandardItem(string)
{
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
