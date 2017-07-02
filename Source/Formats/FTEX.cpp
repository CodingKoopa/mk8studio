#include "FTEX.h"

#include <QImage>

#include "GX2ImageBase.h"

FTEX::FTEX(File* file, quint64 pos) : m_file(file), m_start_offset(pos)
{
}

FTEX::FTEX(const FTEX& other)
    : GX2ImageBase(other), m_file(other.m_file), m_start_offset(other.m_start_offset),
      m_header(other.m_header)
{
  DeepCopyRawImageDataBuffer(other);
}

FTEX& FTEX::operator=(const FTEX& other)
{
  m_file = other.m_file;
  m_start_offset = other.m_start_offset;
  m_header = other.m_header;
  DeepCopyRawImageDataBuffer(other);
  return *this;
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
  m_header.dim = m_file->Read32();
  m_header.width = m_file->Read32();
  m_header.height = m_file->Read32();
  m_header.depth = m_file->Read32();
  m_header.num_mips = m_file->Read32();
  m_header.format = m_file->Read32();
  m_header.aa_mode = m_file->Read32();
  m_header.usage = m_file->Read32();
  m_header.data_length = m_file->Read32();
  m_file->Skip(4);
  m_header.mipmap_length = m_file->Read32();
  m_file->Skip(4);
  m_header.tile_mode = m_file->Read32();
  m_header.swizzle = m_file->Read32();
  m_header.alignment = m_file->Read32();
  m_header.pitch = m_file->Read32();
  m_file->Skip(0x6C);
  m_header.data_offset = m_file->Read32RelativeOffset();
  m_header.mipmap_offset = m_file->Read32RelativeOffset();

  m_base_header = static_cast<ImageHeaderBase>(m_header);
  ResultCode res = SetupInfoStructs();

  return res;
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
  m_file->Write32(m_header.dim);
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
  m_file->Write32RelativeOffset(m_header.mipmap_offset);
  m_file->Seek(m_header.data_offset);
  m_file->WriteBytes(m_raw_image_data.data(), m_header.data_length);
  m_file->Save();
}

quint64 FTEX::GetStart()
{
  return m_start_offset;
}

const FTEX::FTEXHeader& FTEX::GetHeader()
{
  return m_header;
}

void FTEX::SetHeader(const FTEX::FTEXHeader& ftex_header)
{
  m_header = ftex_header;
}

void FTEX::DeepCopyRawImageDataBuffer(const FTEX& other)
{
  if (m_raw_image_data_buffer)
    delete m_raw_image_data_buffer;
  m_raw_image_data_buffer = new char[sizeof(other.m_raw_image_data_buffer)];
  if (other.m_raw_image_data_buffer)
    *m_raw_image_data_buffer = *other.m_raw_image_data_buffer;
}
