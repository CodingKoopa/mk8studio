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
  m_header.bom = m_file->ReadU16();
  try
  {
    m_endian_name = m_endian_names.at(static_cast<Endianness>(m_header.bom));
  }
  catch (std::out_of_range)
  {
    return ResultCode::IncorrectBFRESEndianness;
  }
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

const ResourceDictionary<FMDL>& BFRES::GetFMDLDictionary() const
{
  return m_fmdl_dictionary;
}

void BFRES::SetFMDLDictionary(const ResourceDictionary<FMDL>& dictionary)
{
  m_fmdl_dictionary = dictionary;
}

const ResourceDictionary<FTEX>& BFRES::GetFTEXDictionary() const
{
  return m_ftex_dictionary;
}

void BFRES::SetFTEXDictionary(const ResourceDictionary<FTEX>& dictionary)
{
  m_ftex_dictionary = dictionary;
}

const std::map<BFRES::Endianness, QString>& BFRES::GetEndianNames() const
{
  return m_endian_names;
}

const QString& BFRES::GetEndianName() const
{
  return m_endian_name;
}
