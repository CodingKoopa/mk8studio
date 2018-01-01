#include "Formats/Archives/BFRES.h"

BFRES::BFRES(File* file, quint32 start_offset) : FormatBase(file, start_offset, 0x6C) {}

BFRES::BFRES(const BFRES& other)
    : FormatBase(other), m_header(other.m_header), m_fmdl_dictionary(other.m_fmdl_dictionary)
{
}

BFRES& BFRES::operator=(const BFRES& other)
{
  // This is alright because the pointer won't be deleted until MainWindow is destructed.
  m_file = other.m_file;

  m_header = other.m_header;
  m_fmdl_dictionary = other.m_fmdl_dictionary;
  return *this;
}

ResultCode BFRES::ReadHeader()
{
  const quint32 start_pos = m_file->Pos();

  m_header.magic = m_file->ReadStringASCII(4);
  m_header.unknown_a = m_file->Read8();
  m_header.unknown_b = m_file->Read8();
  m_header.unknown_c = m_file->Read8();
  m_header.unknown_d = m_file->Read8();
  // ACTUAL      WIKI   SKIP()   READ()
  // 00          1      1        read8()
  // 00 00       2      2        read16()
  // 00 00 00 00 4      4        read32()
  quint16 bom = m_file->ReadU16();
  bool endian_info_found = false;
  foreach (const EndianInfo& endian_info, m_endian_info_list)
  {
    if (endian_info.value == static_cast<EndianInfo::Endianness>(bom))
    {
      m_header.endian_info = endian_info;
      endian_info_found = true;
      break;
    }
  }
  if (!endian_info_found)
    return ResultCode::IncorrectBFRESEndianness;
  m_header.unknown_e = m_file->ReadU16();
  m_header.length = m_file->ReadU32();
  m_header.alignment = m_file->ReadU32();
  m_header.file_name_offset = m_file->ReadU32RelativeOffset();
  m_header.string_table_length = m_file->ReadU32();
  m_header.string_table_offset = m_file->ReadU32RelativeOffset();
  for (int i = 0; i < 12; ++i)
    m_header.file_offsets << m_file->ReadU32RelativeOffset();
  for (int i = 0; i < 12; ++i)
    m_header.file_counts << m_file->ReadU16();

  m_header.unknown_f = m_file->ReadU32();

  ResultCode res = CheckHeaderSize(start_pos);
  if (res != ResultCode::Success)
    return res;

  m_file->Seek(m_header.file_name_offset);
  // TODO: apparantly the string table stores the lengths of strings too, so a better way of doing
  // this might be seeking back 4 bytes, reading the length, and calling readstring with that length
  m_header.file_name = m_file->ReadStringASCII(0);
  return res;
}

ResultCode BFRES::ReadDictionaries()
{
  m_fmdl_dictionary = ResourceDictionary<FMDL>(
      m_file, m_header.file_offsets[static_cast<quint32>(GroupType::FMDL)]);
  m_ftex_dictionary = ResourceDictionary<FTEX>(
      m_file, m_header.file_offsets[static_cast<quint32>(GroupType::FTEX)]);

  return ResultCode::Success;
}

const BFRES::Header& BFRES::GetHeader() const
{
  return m_header;
}

void BFRES::SetHeader(const Header& header)
{
  m_header = header;
}

File* BFRES::GetFile() const
{
  return m_file;
}

const ResourceDictionary<FMDL>& BFRES::GetFMDLDictionary()
{
  return m_fmdl_dictionary;
}

void BFRES::SetFMDLDictionary(const ResourceDictionary<FMDL>& dictionary)
{
  m_fmdl_dictionary = dictionary;
}

const ResourceDictionary<FTEX>& BFRES::GetFTEXDictionary()
{
  return m_ftex_dictionary;
}

void BFRES::SetFTEXDictionary(const ResourceDictionary<FTEX>& dictionary)
{
  m_ftex_dictionary = dictionary;
}

const QVector<BFRES::EndianInfo>& BFRES::GetEndianInfoList() const
{
  return m_endian_info_list;
}

const BFRES::EndianInfo& BFRES::GetEndianInfoFromName(const QString& name)
{
  foreach (const EndianInfo& endian_info, m_endian_info_list)
  {
    if (endian_info.name == name)
      return endian_info;
  }
  return m_endian_info_list[0];
}

quint32 BFRES::GetEndianIndexFromInfo(const BFRES::EndianInfo& endian_info)
{
  for (qint32 index = 0; index < m_endian_info_list.size(); ++index)
  {
    if (m_endian_info_list[index].value == endian_info.value)
      return index;
  }
  return 0;
}
