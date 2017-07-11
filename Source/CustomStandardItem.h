#pragma once

#include <functional>

#include <QStandardItem>

class CustomStandardItem : public QStandardItem
{
public:
  CustomStandardItem() = default;
  CustomStandardItem(QString string);
  ~CustomStandardItem();

  void ExecuteFunction();
  void SetFunction(const std::function<void(QString text)>& value = {});

private:
  std::function<void(QString text)> m_function;
};

Q_DECLARE_METATYPE(std::function<void(QString text)>)
