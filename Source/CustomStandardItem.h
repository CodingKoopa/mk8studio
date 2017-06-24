#ifndef CUSTOMSTANDARDITEM_H
#define CUSTOMSTANDARDITEM_H

#include <functional>

#include <QStandardItem>

#include "Nodes/Node.h"

class CustomStandardItem : public QStandardItem
{
public:
  CustomStandardItem(QString string) : QStandardItem(string) {}

  void SetFunction(const std::function<void(QString text)>& value = {}) { m_function = value; }

  void ExecuteFunction()
  {
    if (m_function)
      m_function(text());
  }

private:
  std::function<void(QString text)> m_function;
};

Q_DECLARE_METATYPE(std::function<void(QString text)>)

#endif  // CUSTOMSTANDARDITEM_H
