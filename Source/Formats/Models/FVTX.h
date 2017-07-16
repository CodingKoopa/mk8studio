#pragma once

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"

class FVTX : public FormatBase
{
public:
  FVTX() = default;
  FVTX(File* file, quint32 start_offset);
  ResultCode ReadHeader();

  struct Header
  {
    QString magic;
    quint8 attribute_count;
    quint8 buffer_count;
    quint16 section_index;
    quint32 number_vertices;
    quint8 vertex_skin_count;
    qint32 attribute_array_offset;
    qint32 attribute_index_group_offset;
    qint32 buffer_array_offset;
    quint32 user_pointer_runtime;
  };

  const Header& GetHeader() const;
  void SetHeader(const Header& header);

private:
  File* m_file;
  quint32 m_start_offset;
  Header m_header;
};
