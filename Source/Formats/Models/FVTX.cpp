#include "Formats/Models/FVTX.h"

FVTX::FVTX(File* file, quint32 start_offset) : m_file(file), m_start_offset(start_offset)
{
}

ResultCode FVTX::ReadHeader()
{
  m_file->Seek(m_start_offset);
  m_header.magic = m_file->ReadStringASCII(4);
  m_header.attribute_count = m_file->Read8();
  m_header.buffer_count = m_file->Read8();
  m_header.section_index = m_file->ReadU16();
  m_header.number_vertices = m_file->ReadU32();
  m_header.vertex_skin_count = m_file->Read8();
  m_file->Skip(3);
  m_header.attribute_array_offset = m_file->ReadS32RelativeOffset();
  m_header.attribute_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.buffer_array_offset = m_file->ReadS32RelativeOffset();
  m_header.user_pointer_runtime = m_file->ReadU32();

  if (m_file->Pos() - m_start_offset != 0x20)
    return ResultCode::IncorrectHeaderSize;

  return ResultCode::Success;
}

const FVTX::Header& FVTX::GetHeader() const
{
  return m_header;
}

void FVTX::SetHeader(const Header& header)
{
  m_header = header;
}
