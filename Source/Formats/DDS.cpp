#include "DDS.h"
#include "FileBase.h"
#include "GX2ImageBase.h"

DDS::DDS(QByteArray* image_data) : m_image_data(image_data)
{
}

// TODO: Add uncompressed support
int DDS::MakeHeader(quint32 width, quint32 height, quint32 depth, quint32 num_mips, bool compressed,
                    GX2ImageBase::FormatInfo format_info)
{
  if (compressed)
  {
    switch (format_info.format)
    {
    case GX2ImageBase::Format::BC1:
    {
      m_block_size = 8;
      break;
    }
    default:
    {
      m_block_size = 16;
      break;
    }
    }
  }

  m_header.magic = "DDS ";

  m_header.header_size = 124;

  m_header.flags = DDS_FLAG_CAPS | DDS_FLAG_HEIGHT | DDS_FLAG_WIDTH | DDS_FLAG_PIXELFORMAT;
  if (num_mips > 1)
    m_header.flags |= DDS_FLAG_HASMIPMAPS;
  if (compressed)
    m_header.flags |= DDS_FLAG_LINEARSIZE;

  m_header.height = height;

  m_header.width = width;

  if (compressed)
  {
    m_header.pitch_or_linear_size = ((width + 3) >> 2) * ((height + 3) >> 2);
    m_header.pitch_or_linear_size *= m_block_size;
  }
  else
    m_header.pitch_or_linear_size = 0;

  m_header.depth = depth;

  m_header.num_mips = 1;

  m_header.reserved1[11] = {0};

  m_header.pixel_format.formatSize = 32;

  if (compressed)
    m_header.pixel_format.pixelFlags = DDS_PIX_FLAG_COMPRESSED;
  else
    m_header.pixel_format.pixelFlags = DDS_PIX_FLAG_UNCOMPRESSEDRGB;

  if (compressed)
  {
    switch (format_info.format)
    {
    case GX2ImageBase::Format::BC1:
      m_header.pixel_format.magic = "DXT1";
      break;
    case GX2ImageBase::Format::BC4:
      if (format_info.type == GX2ImageBase::FormatInfo::Type::UNorm)
        m_header.pixel_format.magic = "BC4U";
      else
        m_header.pixel_format.magic = "BC4S";
      break;
    case GX2ImageBase::Format::BC5:

      if (format_info.type == GX2ImageBase::FormatInfo::Type::UNorm)
        m_header.pixel_format.magic = "BC5U";
      else
        m_header.pixel_format.magic = "BC5S";
      break;
    default:
      m_header.pixel_format.magic = QString();
      break;
    }
  }

  if (compressed)
  {
    m_header.pixel_format.rgb_bit_count = 0;
    m_header.pixel_format.red_bit_mask = 0;
    m_header.pixel_format.green_bit_mask = 0;
    m_header.pixel_format.blue_bit_mask = 0;
    m_header.pixel_format.alpha_bit_mask = 0;
  }

  m_header.complexity_flags = DDS_COMP_FLAG_TEXTURE;
  if (num_mips > 1)
    m_header.complexity_flags |= DDS_COMP_FLAG_COMPLEX | DDS_COMP_FLAG_HASMIPMAPS;

  // TODO: could this be important?
  m_header.caps2 = 0;

  m_header.caps3 = 0;

  m_header.caps4 = 0;

  m_header.reserved2 = 0;

  return 0;
}

int DDS::WriteFile(QString path)
{
  FileBase file(path);
  file.SetByteOrder(QDataStream::LittleEndian);
  file.WriteStringASCII(m_header.magic, 4);
  file.Write32(m_header.header_size);
  file.Write32(m_header.flags);
  file.Write32(m_header.height);
  file.Write32(m_header.width);
  file.Write32(m_header.pitch_or_linear_size);
  file.Write32(m_header.depth);
  file.Write32(m_header.num_mips);
  file.Skip(44);
  file.Write32(m_header.pixel_format.formatSize);
  file.Write32(m_header.pixel_format.pixelFlags);
  file.WriteStringASCII(m_header.pixel_format.magic, 4);
  file.Write32(m_header.pixel_format.rgb_bit_count);
  file.Write32(m_header.pixel_format.red_bit_mask);
  file.Write32(m_header.pixel_format.green_bit_mask);
  file.Write32(m_header.pixel_format.blue_bit_mask);
  file.Write32(m_header.pixel_format.alpha_bit_mask);
  file.Write32(m_header.complexity_flags);
  file.Write32(m_header.caps2);
  file.Write32(m_header.caps3);
  file.Write32(m_header.caps4);
  file.Write32(m_header.reserved2);
  int ret = file.WriteBytes(m_image_data->data(), m_image_data->size());
  file.Save();
  return ret;
}
