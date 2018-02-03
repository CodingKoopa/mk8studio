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
  void SetFunction(const std::function<void(quint32 index)>& function);
  void SetFunction(const std::function<void(QString value)>& function);
  //  void SetFunction(const std::function<void(quint32 index)>& function);

private:
  // TODO: This isn't an amazing solution for the two different types of functions. Maybe this class
  // could be templated for each?
  enum class FunctionType
  {
    None,
    Index,
    Value
  } m_function_type = FunctionType::None;
  std::function<void(quint32)> m_index_function;
  std::function<void(QString)> m_value_function;
};
