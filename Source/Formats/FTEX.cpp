#include <QImage>

#include "FTEX.h"
#include "GX2ImageBase.h"

int FTEX::ReadHeader()
{
  m_header = new FTEXHeader;
  m_file->Seek(m_start_offset);
  m_header->magic = m_file->ReadStringASCII(4);
  m_header->dim = m_file->Read32();
  m_header->width = m_file->Read32();
  m_header->height = m_file->Read32();
  m_header->depth = m_file->Read32();
  m_header->num_mips = m_file->Read32();
  m_header->format = m_file->Read32();
  m_header->aa_mode = m_file->Read32();
  m_header->usage = m_file->Read32();
  m_header->data_length = m_file->Read32();
  m_file->Skip(4);
  m_header->mipmap_length = m_file->Read32();
  m_file->Skip(4);
  m_header->tile_mode = m_file->Read32();
  m_header->swizzle = m_file->Read32();
  m_header->alignment = m_file->Read32();
  m_header->pitch = m_file->Read32();
  m_file->Skip(0x6C);
  m_header->data_offset = m_file->Pos() + m_file->Read32();
  m_header->mipmap_offset = m_file->Pos() + m_file->Read32();
  //  m_file->Skip(8);

  m_base_header = static_cast<ImageHeaderBase*>(m_header);
  SetupInfoStructs();

  if (m_file->Pos() != m_start_offset + 0xC0)
    return -1;

  return 0;
}

ResultCode FTEX::ReadImage()
{
  m_file->Seek(m_header->data_offset);
  char* buffer = m_file->ReadBytes(m_header->data_length);
  m_raw_image_data = new QByteArray();
  m_raw_image_data->append(buffer, m_header->data_length);
  delete[] buffer;

  return ReadImageFromData();
}

void FTEX::InjectImage()
{
  m_file->Seek(m_start_offset);
  m_file->WriteStringASCII(m_header->magic, 4);
  m_file->Write32(m_header->dim);
  m_file->Write32(m_header->width);
  m_file->Write32(m_header->height);
  m_file->Write32(m_header->depth);
  m_file->Write32(m_header->num_mips);
  m_file->Write32(m_header->format);
  m_file->Write32(m_header->aa_mode);
  m_file->Write32(m_header->usage);
  m_file->Write32(m_header->data_length);
  m_file->Skip(4);
  m_file->Write32(m_header->mipmap_length);
  m_file->Skip(4);
  m_file->Write32(m_header->tile_mode);
  m_file->Write32(m_header->swizzle);
  m_file->Write32(m_header->alignment);
  m_file->Write32(m_header->pitch);
  m_file->Skip(0x6C);
  m_file->Write32(m_header->data_offset - m_file->Pos());
  m_file->Write32(m_header->mipmap_offset);
  m_file->Seek(m_header->data_offset);
  m_file->WriteBytes(m_raw_image_data->data(), m_header->data_length);
  m_file->Save();
}

quint64 FTEX::GetStart()
{
  return m_start_offset;
}
