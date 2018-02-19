#include "Formats/Textures/FTEX.h"

#include <QImage>

#include "Formats/Textures/GX2ImageBase.h"

FTEX::FTEX(std::shared_ptr<File> file, quint64 start_offset)
    : GX2ImageBase(file, start_offset, HEADER_SIZE)
{
}

FTEX::~FTEX()
{
  if (m_raw_image_data_buffer)
    delete m_raw_image_data_buffer;
}

ResultCode FTEX::ReadHeader()
{
  m_file->Seek(m_start_offset);
  m_header.magic = m_file->ReadStringASCII(4);
  m_header.dimension = m_file->ReadU32();
  m_header.width = m_file->ReadU32();
  m_header.height = m_file->ReadU32();
  m_header.depth = m_file->ReadU32();
  m_header.num_mips = m_file->ReadU32();
  m_header.format = m_file->ReadU32();
  m_header.aa_mode = m_file->ReadU32();
  m_header.usage = m_file->ReadU32();
  m_header.data_length = m_file->ReadU32();
  m_header.data_offset_runtime = m_file->ReadU32();
  m_header.mipmap_length = m_file->ReadU32();
  m_header.mipmap_offset_runtime = m_file->ReadU32();
  m_header.tile_mode = m_file->ReadU32();
  m_header.swizzle = m_file->ReadU32();
  m_header.alignment = m_file->ReadU32();
  m_header.pitch = m_file->ReadU32();
  for (int i = 0; i < 13; ++i)
    m_header.mipmap_offsets << m_file->ReadU32();
  m_header.first_mipmap = m_file->ReadU32();
  m_header.num_mips_alt = m_file->ReadU32();
  m_header.first_slice = m_file->ReadU32();
  m_header.num_slices = m_file->ReadU32();
  m_header.red_channel_component = m_file->Read8();
  m_header.green_channel_component = m_file->Read8();
  m_header.blue_channel_component = m_file->Read8();
  m_header.alpha_channel_component = m_file->Read8();
  for (int i = 0; i < 5; ++i)
    m_header.registers << m_file->ReadU32();
  m_header.texture_handle_runtime = m_file->ReadU32();
  m_header.array_length = m_file->ReadU32();
  m_header.file_name_offset = m_file->ReadS32RelativeOffset();
  m_header.file_path_offset = m_file->ReadS32RelativeOffset();
  m_header.data_offset = m_file->ReadS32RelativeOffset();
  m_header.mipmap_section_offset = m_file->ReadS32RelativeOffset();
  m_header.user_data_index_group_offset = m_file->ReadS32RelativeOffset();
  m_header.user_data_entry_count = m_file->ReadU16();
  m_file->Skip(2);

  // Fix the mipmap offsets to be absolute.
  for (int i = 0; i < m_header.mipmap_offsets.size(); ++i)
  {
    if (m_header.mipmap_offsets[i])
      m_header.mipmap_offsets[i] += m_header.mipmap_section_offset;
  }

  ResultCode res = CheckHeaderSize(m_start_offset);
  if (res != ResultCode::Success)
    return res;

  m_base_header = static_cast<ImageHeaderBase>(m_header);

  return SetupInfo();
}

ResultCode FTEX::ReadImage()
{
  m_file->Seek(m_header.data_offset);
  m_raw_image_data_buffer = new char[m_header.data_length];
  m_file->ReadBytes(m_header.data_length, m_raw_image_data_buffer);
  m_raw_image_data.append(m_raw_image_data_buffer, m_header.data_length);

  return ReadImageFromData();
}

void FTEX::InjectImage()
{
  m_file->Seek(m_start_offset);
  m_file->WriteStringASCII(m_header.magic, 4);
  m_file->Write32(m_header.dimension);
  m_file->Write32(m_header.width);
  m_file->Write32(m_header.height);
  m_file->Write32(m_header.depth);
  m_file->Write32(m_header.num_mips);
  m_file->Write32(m_header.format);
  m_file->Write32(m_header.aa_mode);
  m_file->Write32(m_header.usage);
  m_file->Write32(m_header.data_length);
  m_file->Skip(4);
  m_file->Write32(m_header.mipmap_length);
  m_file->Skip(4);
  m_file->Write32(m_header.tile_mode);
  m_file->Write32(m_header.swizzle);
  m_file->Write32(m_header.alignment);
  m_file->Write32(m_header.pitch);
  m_file->Skip(0x6C);
  m_file->Write32RelativeOffset(m_header.data_offset);
  m_file->Write32RelativeOffset(m_header.mipmap_section_offset);
  m_file->Seek(m_header.data_offset);
  m_file->WriteBytes(m_raw_image_data.data(), m_header.data_length);
  m_file->Save();
}

const QVector<QString> FTEX::GetComponentNameList() const
{
  return m_component_name_list;
}

quint8 FTEX::GetComponentIDFromName(const QString& name) const
{
  for (quint8 id = 0; id < m_component_name_list.size(); ++id)
  {
    if (name == m_component_name_list[id])
      return id;
  }
  return 0;
}

quint64 FTEX::GetStart() const
{
  return m_start_offset;
}

const FTEX::Header& FTEX::GetHeader() const
{
  return m_header;
}

void FTEX::SetHeader(const FTEX::Header& ftex_header)
{
  m_header = ftex_header;
  m_base_header = static_cast<ImageHeaderBase>(m_header);
}
