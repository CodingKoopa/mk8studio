#pragma once

#include <memory>

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"
#include "Formats/Models/FVTX.h"

class FMDL : public FormatBase
{
public:
  FMDL(std::shared_ptr<File> file, quint32 start_offset = 0);
  ResultCode ReadHeader();
  ResultCode ReadFVTXArray();

  struct Header
  {
    QString magic;
    qint32 file_name_offset;
    qint32 file_path_offset;
    qint32 fskl_offset;
    qint32 fvtx_array_offset;
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

  typedef std::shared_ptr<std::vector<std::shared_ptr<FVTX>>> FVTXList;
  FVTXList GetFVTXList() const;

private:
  static constexpr quint32 HEADER_SIZE = 0x30;

  Header m_header;

  FVTXList m_fvtx_list;
};
