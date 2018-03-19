#include "Formats/Archives/BFRES.h"

BFRES::BFRES(std::shared_ptr<File> file, quint32 start_offset)
    : FormatBase(file, start_offset, HEADER_SIZE)
{
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
  m_name = m_file->ReadStringASCII(0);
  return res;
}

ResultCode BFRES::ReadDictionaries()
{
  m_fmdl_dictionary = std::make_shared<ResourceDictionary<FMDL>>(
      m_file, m_header.file_offsets[static_cast<quint32>(GroupType::FMDL)]);
  m_ftex_dictionary = std::make_shared<ResourceDictionary<FTEX>>(
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

std::map<BFRES::Endianness, QString> BFRES::GetEndianNames()
{
  return m_endian_names;
}

const QString& BFRES::GetEndianName() const
{
  return m_endian_name;
}

std::shared_ptr<ResourceDictionary<FMDL>> BFRES::GetFMDLDictionary() const
{
  return m_fmdl_dictionary;
}

std::shared_ptr<ResourceDictionary<FTEX>> BFRES::GetFTEXDictionary() const
{
  return m_ftex_dictionary;
}
