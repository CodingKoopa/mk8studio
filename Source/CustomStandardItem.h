#pragma once

#include <functional>

#include <QStandardItem>

#include "Nodes/Node.h"

class CustomStandardItem : public QStandardItem
{
public:
  CustomStandardItem(QString string) : QStandardItem(string) {}

  void ExecuteFunction();

  void SetFunction(const std::function<void(QString text)>& value = {});

private:
  std::function<void(QString text)> m_function;
};

Q_DECLARE_METATYPE(std::function<void(QString text)>)
