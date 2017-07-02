#include "FormatBase.h"

const QString& FormatBase::GetName()
{
  return m_name;
}

void FormatBase::SetName(const QString& value)
{
  m_name = value;
}

const QString& FormatBase::GetPath()
{
  return m_path;
}

void FormatBase::SetPath(const QString& value)
{
  m_path = value;
}
