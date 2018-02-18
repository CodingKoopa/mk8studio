#pragma once

#include <QString>

#include "Common.h"
#include "File.h"

class FormatBase
{
  // When saving is implemented, this class will be more useful.
public:
  FormatBase(File* file = nullptr, quint32 start_offset = 0, quint32 header_size = 0);
  FormatBase(const FormatBase& other);

  const QString& GetName() const;
  void SetName(const QString& name);

  const QString& GetPath() const;
  void SetPath(const QString& path);

  quint32 GetHeaderSize() const;

protected:
  ResultCode CheckHeaderSize(quint32 start_pos, quint32 header_size = 0);

  File* m_file;
  quint32 m_start_offset;
  quint32 m_header_size;
  QString m_name;
  QString m_path;
};
