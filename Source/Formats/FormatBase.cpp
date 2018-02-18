#include "Formats/FormatBase.h"

FormatBase::FormatBase(File* file, quint32 start_offset, quint32 header_size)
    : m_file(file), m_start_offset(start_offset), m_header_size(header_size)
{
}

FormatBase::FormatBase(const FormatBase& other)
    : m_file(other.m_file), m_start_offset(other.m_start_offset),
      m_header_size(other.m_header_size), m_name(other.m_name), m_path(other.m_path)
{
}

const QString& FormatBase::GetName() const
{
  return m_name;
}

void FormatBase::SetName(const QString& name)
{
  m_name = name;
}

const QString& FormatBase::GetPath() const
{
  return m_path;
}

void FormatBase::SetPath(const QString& path)
{
  m_path = path;
}

quint32 FormatBase::GetHeaderSize() const
{
  return m_header_size;
}

ResultCode FormatBase::CheckHeaderSize(quint32 start_pos, quint32 header_size)
{
  if (!header_size)
  {
    if (m_header_size)
      header_size = m_header_size;
    else
      return ResultCode::IncorrectHeaderSize;
  }

  if (m_file->Pos() - start_pos != header_size)
    return ResultCode::IncorrectHeaderSize;
  else
    return ResultCode::Success;
}
