#include "Formats/Models/FMDL.h"

FMDL::FMDL(std::shared_ptr<File> file, quint32 start_offset)
    : FormatBase(file, start_offset, HEADER_SIZE),
      m_fvtx_list(std::make_shared<std::vector<std::shared_ptr<FVTX>>>())
{
}

ResultCode FMDL::ReadHeader()
{
  m_file->Seek(m_start_offset);
  const quint32 start_pos = m_file->Pos();
  m_header.magic = m_file->ReadStringASCII(4);
  m_header.file_name_offset = m_file->ReadS32RelativeOffset();
  m_header.file_path_offset = m_file->ReadS32RelativeOffset();
  m_header.fskl_offset = m_file->ReadS32RelativeOffset();
  m_header.fvtx_array_offset = m_file->ReadS32RelativeOffset();
  m_header.fshp_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.fmat_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.user_data_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.fvtx_count = m_file->ReadU16();
  m_header.fshp_count = m_file->ReadU16();
  m_header.fmat_count = m_file->ReadU16();
  m_header.user_data_entry_count = m_file->ReadU16();
  m_header.num_vertices = m_file->ReadU32();
  m_header.user_pointer_runtime = m_file->ReadU32();

  return CheckHeaderSize(start_pos);
}

ResultCode FMDL::ReadFVTXArray()
{
  m_fvtx_list->clear();

  m_file->Seek(m_header.fvtx_array_offset);
  for (quint16 index = 0; index < m_header.fvtx_count; ++index)
  {
    std::shared_ptr<FVTX> fvtx = std::make_shared<FVTX>(m_file, m_file->Pos());
    m_file->Skip(fvtx->GetHeaderSize());
    m_fvtx_list->push_back(std::move(fvtx));
  }
  return ResultCode::Success;
}

const FMDL::Header& FMDL::GetHeader() const
{
  return m_header;
}

void FMDL::SetHeader(const Header& header)
{
  m_header = header;
}

FMDL::FVTXList FMDL::GetFVTXList() const
{
  return m_fvtx_list;
}
