#include "FormatBase.h"

const QString& FormatBase::GetName() const
{
  return m_name;
}

void FormatBase::SetName(const QString& value)
{
  m_name = value;
}

const QString& FormatBase::GetPath() const
{
  return m_path;
}

void FormatBase::SetPath(const QString& value)
{
  m_path = value;
}
