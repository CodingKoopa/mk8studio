#include "Formats/Models//FMDL.h"

FMDL::FMDL(File* file, quint32 start_offset) : m_file(file), m_start_offset(start_offset)
{
}

ResultCode FMDL::ReadHeader()
{
  m_file->Seek(m_start_offset);
  m_header.magic = m_file->ReadStringASCII(4);
  m_header.file_name_offset = m_file->ReadS32RelativeOffset();
  m_header.file_path_offset = m_file->ReadS32RelativeOffset();
  m_header.fskl_offset = m_file->ReadS32RelativeOffset();
  m_header.ftex_array_offset = m_file->ReadS32RelativeOffset();
  m_header.fshp_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.fmat_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.user_data_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.fvtx_count = m_file->ReadU16();
  m_header.fshp_count = m_file->ReadU16();
  m_header.fmat_count = m_file->ReadU16();
  m_header.user_data_entry_count = m_file->ReadU16();
  m_header.num_vertices = m_file->ReadU32();
  m_header.user_pointer_runtime = m_file->ReadU32();

  if (m_file->Pos() - m_start_offset != 0x30)
    return ResultCode::IncorrectHeaderSize;

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
