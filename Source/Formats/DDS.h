#ifndef DDS_H
#define DDS_H

#include <QByteArray>

#include "Common.h"
#include "FormatBase.h"
#include "GX2ImageBase.h"

class DDS : public FormatBase
{
public:
  DDS() : m_image_data(new QByteArray()) {}

  struct PixelFormat
  {
    quint32 format_size;
    quint32 pixel_flags;
    QString four_cc;
    quint32 rgb_bit_count;
    quint32 red_bit_mask;
    quint32 green_bit_mask;
    quint32 blue_bit_mask;
    quint32 alpha_bit_mask;
  };

  struct DDSHeader
  {
    QString magic;
    quint32 header_size;
    quint32 flags;
    quint32 height;
    quint32 width;
    quint32 pitch_or_linear_size;
    quint32 depth;
    quint32 num_mips;
    quint32 reserved1[11];
    PixelFormat pixel_format;
    quint32 complexity_flags;
    quint32 caps2;
    quint32 caps3;
    quint32 caps4;
    quint32 reserved2;
  };

  ResultCode ReadFile();
  int WriteFile(quint32 width, quint32 height, quint32 depth, quint32 num_mips, bool compressed,
                GX2ImageBase::FormatInfo format_info);

  QByteArray* GetImageData() { return m_image_data; }
  void SetImageData(QByteArray* image_data) { m_image_data = image_data; }

private:
  enum class ImageFlag
  {
    Caps = 0x1,
    Height = 0x2,
    Width = 0x4,
    Pitch = 0x8,
    PixelFormat = 0x1000,
    MipMaps = 0x20000,
    LinearSize = 0x80000,
    Depth = 0x800000,

    RequiredFlags = Caps | Height | Width | PixelFormat
  };

  enum ComplexityFlag
  {
    DDS_COMP_FLAG_COMPLEX = 0x8,
    DDS_COMP_FLAG_TEXTURE = 0x1000,
    DDS_COMP_FLAG_HASMIPMAPS = 0x400000
  };

  enum PixelFlag
  {
    DDS_PIX_FLAG_HASALPHA = 0x1,
    DDS_PIX_FLAG_UNCOMPESSEDALPHA = 0x2,
    DDS_PIX_FLAG_COMPRESSED = 0x4,
    DDS_PIX_FLAG_UNCOMPRESSEDRGB = 0x40,
    DDS_PIX_FLAG_UNCOMPRESSEDYUV = 0x200,
    DDS_PIX_FLAG_LUMINANCE = 0x20000,
  };

  quint32 CalculateLinearSize(quint32 block_size)
  {
    return ((m_header.width + 3) >> 2) * ((m_header.height + 3) >> 2) * block_size;
  }

  QByteArray* m_image_data;
  DDSHeader m_header;
};

#endif  // DDS_H
