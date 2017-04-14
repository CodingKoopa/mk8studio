#include <QImage>

#include "FTEX.h"
#include "GX2ImageBase.h"

int FTEX::ReadHeader()
{
  m_ftex_header = new FTEXHeader;
  file->Seek(start);
  m_ftex_header->magic = file->ReadStringASCII(4);
  m_ftex_header->dim = file->Read32();
  m_ftex_header->width = file->Read32();
  m_ftex_header->height = file->Read32();
  m_ftex_header->depth = file->Read32();
  m_ftex_header->num_mips = file->Read32();
  m_ftex_header->format = static_cast<Format>(file->Read32());
  m_ftex_header->aa_mode = file->Read32();
  m_ftex_header->usage = file->Read32();
  m_ftex_header->data_length = file->Read32();
  file->Skip(4);
  m_ftex_header->mipSize = file->Read32();
  file->Skip(4);
  m_ftex_header->tile_mode = static_cast<TileMode>(file->Read32());
  m_ftex_header->swizzle = file->Read32();
  m_ftex_header->alignment = file->Read32();
  m_ftex_header->pitch = file->Read32();
  file->Skip(0x6C);
  m_ftex_header->data_offset = file->Pos() + file->Read32();
  m_ftex_header->mipmap_offset = file->Pos() + file->Read32();
  file->Skip(8);

  m_header = static_cast<ImageHeader*>(m_ftex_header);

  if (file->Pos() != start + 0xC0)
    return -1;

  return 0;
}

ResultCode FTEX::ReadImageData()
{
  file->Seek(m_ftex_header->data_offset);
  char* buffer = file->ReadBytes(m_header->data_length);
  raw_image_data.append(buffer, m_header->data_length);
  raw_image_data.resize(m_header->data_length);
  delete[] buffer;

  return ReadImageFromData();
}

QImage* FTEX::GetImage()
{
  QImage* image =
      new QImage(m_header->width, m_header->height, QImage::Format_RGBA8888_Premultiplied);
  image->fill(Qt::red);

  return image;
}

FTEX::FTEXHeader FTEX::GetHeader()
{
  return *m_ftex_header;
}

quint64 FTEX::GetStart()
{
  return start;
}
