#ifndef FMDL_H
#define FMDL_H

#include "Common.h"
#include "File.h"
#include "FormatBase.h"

class FMDL : public FormatBase
{
public:
  FMDL(File* file, quint32 start_offset);
  ResultCode ReadHeader();

  struct Header
  {
    QString magic;
    qint32 file_name_offset;
    qint32 file_path_offset;
    qint32 fskl_offset;
    qint32 ftex_array_offset;
    qint32 fshp_index_group_offset;
    qint32 fmat_index_group_offset;
    qint32 user_data_index_group_offset;
    quint16 fvtx_count;
    quint16 fshp_count;
    quint16 fmat_count;
    quint16 user_data_entry_count;
    quint32 num_vertices;
    quint32 user_pointer_runtime;
  };

  const Header& GetHeader() const;
  void SetHeader(const Header& header);

private:
  File* m_file;
  quint32 m_start_offset;
  Header m_header;
};

#endif  // FMDL_H
