#include "DDS.h"

#include "FileBase.h"
#include "GX2ImageBase.h"

DDS::~DDS()
{
  if (m_image_data_buffer)
    delete m_image_data_buffer;
}

ResultCode DDS::ReadFile()
{
  FileBase dds_file(m_path);
  dds_file.SetByteOrder(QDataStream::LittleEndian);
  m_header.magic = dds_file.ReadStringASCII(4);
  m_header.header_size = dds_file.Read32();
  m_header.flags = dds_file.Read32();
  m_header.height = dds_file.Read32();
  m_header.width = dds_file.Read32();
  m_header.pitch_or_linear_size = dds_file.Read32();
  m_header.depth = dds_file.Read32();
  m_header.num_mips = dds_file.Read32();
  dds_file.Skip(44);
  m_header.pixel_format.format_size = dds_file.Read32();
  m_header.pixel_format.pixel_flags = dds_file.Read32();
  m_header.pixel_format.four_cc = dds_file.ReadStringASCII(4);
  m_header.pixel_format.rgb_bit_count = dds_file.Read32();
  m_header.pixel_format.red_bit_mask = dds_file.Read32();
  m_header.pixel_format.green_bit_mask = dds_file.Read32();
  m_header.pixel_format.blue_bit_mask = dds_file.Read32();
  m_header.pixel_format.alpha_bit_mask = dds_file.Read32();
  m_header.complexity_flags = dds_file.Read32();
  m_header.caps2 = dds_file.Read32();
  m_header.caps3 = dds_file.Read32();
  m_header.caps4 = dds_file.Read32();
  m_header.reserved2 = dds_file.Read32();
  // For a COMPRESSED texture with no mipmaps, the layout might look something like this:
  // Magic:             0x00004
  // Header Size:       0x0007c
  // Linear Size:       0x20000
  // File Size:         0x20080
  // This will change if there are mipmaps, or the texture is uncompressed.
  // If the texture is compressed
  quint32 image_data_size = 0;
  // Some tools leave this blank, so we'll have to calculate it ourselves when that happens.
  if (!m_header.pitch_or_linear_size)
  {
    if (m_header.flags & static_cast<quint32>(ImageFlag::LinearSize))
    {
      if (m_header.pixel_format.four_cc == "DXT1")
        image_data_size = CalculateLinearSize(8);
      else
        image_data_size = CalculateLinearSize(16);
    }
    else
    {
      /* Calculate From Pitch */
    }
  }
  else
  {
    if (m_header.flags & static_cast<quint32>(ImageFlag::LinearSize))
      image_data_size = m_header.pitch_or_linear_size;
    else
    {
      /* Calculate From Pitch */
    }
  }

  m_image_data_buffer = new char[image_data_size];
  dds_file.ReadBytes(image_data_size, m_image_data_buffer);
  m_image_data.setRawData(m_image_data_buffer, image_data_size);

  return ResultCode::Success;
}

// TODO: Add uncompressed support
int DDS::WriteFile(quint32 width, quint32 height, quint32 depth, quint32 num_mips,
                   quint32 element_size, GX2ImageBase::FormatInfo format_info,
                   GX2ImageBase::SharedFormatInfo shared_format_info)
{
  FileBase dds_file(m_path);
  dds_file.SetByteOrder(QDataStream::LittleEndian);

  // Magic
  m_header.magic = "DDS ";
  dds_file.WriteStringASCII(m_header.magic, 4);

  // Header size
  m_header.header_size = 124;
  dds_file.Write32(m_header.header_size);

  // Flags
  m_header.flags = static_cast<quint32>(ImageFlag::RequiredFlags);
  if (num_mips > 1)
    m_header.flags |= static_cast<quint32>(ImageFlag::MipMaps);
  if (shared_format_info.compressed)
    m_header.flags |= static_cast<quint32>(ImageFlag::LinearSize);
  else
    m_header.flags |= static_cast<quint32>(ImageFlag::Pitch);
  dds_file.Write32(m_header.flags);

  // Height
  m_header.height = height;
  dds_file.Write32(m_header.height);

  // Width
  m_header.width = width;
  dds_file.Write32(m_header.width);

  // Pitch for uncompressed textures, or linear size for compressed textures.
  if (shared_format_info.compressed)
    m_header.pitch_or_linear_size = CalculateLinearSize(element_size);
  else
    // TODO
    m_header.pitch_or_linear_size = 0;
  dds_file.Write32(m_header.pitch_or_linear_size);

  // Depth (Untested with depth textures.)
  m_header.depth = depth;
  dds_file.Write32(m_header.depth);

  // TODO: Mipmaps aren't deswizzled, and therefore not written.
  m_header.num_mips = 1;
  dds_file.Write32(m_header.num_mips);

  // Unused
  m_header.reserved1[11] = {0};
  dds_file.Skip(44);

  // Format header size
  m_header.pixel_format.format_size = 32;
  dds_file.Write32(m_header.pixel_format.format_size);

  // Pixel flags
  if (shared_format_info.compressed)
    m_header.pixel_format.pixel_flags = static_cast<quint32>(PixelFlag::IsCompressed);
  else
    m_header.pixel_format.pixel_flags = static_cast<quint32>(PixelFlag::IsUncompressedRGB);
  dds_file.Write32(m_header.pixel_format.pixel_flags);

  // Four character code
  if (shared_format_info.compressed)
  {
    switch (format_info.format)
    {
    case GX2ImageBase::Format::BC1:
      m_header.pixel_format.four_cc = "DXT1";
      break;
    case GX2ImageBase::Format::BC4:
      if (format_info.type == GX2ImageBase::FormatInfo::Type::UNorm)
        m_header.pixel_format.four_cc = "BC4U";
      else
        m_header.pixel_format.four_cc = "BC4S";
      break;
    case GX2ImageBase::Format::BC5:

      if (format_info.type == GX2ImageBase::FormatInfo::Type::UNorm)
        m_header.pixel_format.four_cc = "BC5U";
      else
        m_header.pixel_format.four_cc = "BC5S";
      break;
    default:
      m_header.pixel_format.four_cc = QString();
      break;
    }
  }
  dds_file.WriteStringASCII(m_header.pixel_format.four_cc, 4);

  // RGBA bitmasks, not used for compressed images
  if (shared_format_info.compressed)
  {
    m_header.pixel_format.rgb_bit_count = 0;
    m_header.pixel_format.red_bit_mask = 0;
    m_header.pixel_format.green_bit_mask = 0;
    m_header.pixel_format.blue_bit_mask = 0;
    m_header.pixel_format.alpha_bit_mask = 0;
  }
  dds_file.Write32(m_header.pixel_format.rgb_bit_count);
  dds_file.Write32(m_header.pixel_format.red_bit_mask);
  dds_file.Write32(m_header.pixel_format.green_bit_mask);
  dds_file.Write32(m_header.pixel_format.blue_bit_mask);
  dds_file.Write32(m_header.pixel_format.alpha_bit_mask);

  // Complexity flags
  m_header.complexity_flags = static_cast<quint32>(ComplexityFlag::Texture);
  if (num_mips > 1)
    m_header.complexity_flags |= static_cast<quint32>(ComplexityFlag::Complex) |
                                 static_cast<quint32>(ComplexityFlag::HasMipMaps);
  dds_file.Write32(m_header.complexity_flags);

  m_header.caps2 = 0;
  dds_file.Write32(m_header.caps2);

  m_header.caps3 = 0;
  dds_file.Write32(m_header.caps3);

  m_header.caps4 = 0;
  dds_file.Write32(m_header.caps4);

  m_header.reserved2 = 0;
  dds_file.Write32(m_header.reserved2);

  // Image data
  int image_data_bytes_written = dds_file.WriteBytes(m_image_data.data(), m_image_data.size());

  dds_file.Save();
  return image_data_bytes_written;
}

const QByteArray& DDS::GetImageData()
{
  return m_image_data;
}

void DDS::SetImageData(const QByteArray& image_data)
{
  m_image_data = image_data;
}
