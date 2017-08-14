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

ResultCode FVTX::ReadAttributes()
{
  m_attribute_list.resize(m_header.attribute_count);
  m_file->Seek(m_header.attribute_array_offset);
  // Read the attribute headers.
  for (quint8 attribute = 0; attribute < m_header.attribute_count; ++attribute)
  {
    m_attribute_list[attribute].name_offset = m_file->ReadS32RelativeOffset();
    m_attribute_list[attribute].buffer_index = m_file->Read8();
    m_file->Skip(1);
    m_attribute_list[attribute].buffer_offset = m_file->ReadU16();
    m_attribute_list[attribute].format = m_file->ReadU32();
  }
  // Populate the attribute info lists;
  for (quint8 attribute = 0; attribute < m_header.attribute_count; ++attribute)
  {
    m_file->Seek(m_attribute_list[attribute].name_offset);
    // TODO: Use string table.
    m_attribute_list[attribute].name = m_file->ReadStringASCII(3);
    // Default value.
    m_attribute_list[attribute].name_info = m_attribute_name_info_list[0];
    foreach (const FVTX::AttributeNameInfo& name_info, m_attribute_name_info_list)
    {
      if (name_info.internal_name == m_attribute_list[attribute].name)
      {
        m_attribute_list[attribute].name_info = name_info;
        break;
      }
    }
  }

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

const QVector<FVTX::Attribute>& FVTX::GetAttributeList()
{
  return m_attribute_list;
}
