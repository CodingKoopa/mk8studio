#pragma once

#include <QByteArray>

#include "Common.h"
#include "FormatBase.h"
#include "GX2ImageBase.h"
#include "NonCopyable.h"

class DDS : public FormatBase, NonCopyable
{
public:
  ~DDS();

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
  int WriteFile(quint32 width, quint32 height, quint32 depth, quint32 num_mips,
                quint32 element_size, GX2ImageBase::FormatInfo format_info,
                GX2ImageBase::SharedFormatInfo shared_format_info);

  const QByteArray& GetImageData();
  void SetImageData(const QByteArray& image_data);

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

  enum class ComplexityFlag
  {
    Complex = 0x8,
    Texture = 0x1000,
    HasMipMaps = 0x400000
  };

  enum class PixelFlag
  {
    HasAlpha = 0x1,
    HasUncompressedAlpha = 0x2,
    IsCompressed = 0x4,
    IsUncompressedRGB = 0x40,
    IsUncompressedYUV = 0x200,
    HasLuminance = 0x20000,
  };

  quint32 CalculateLinearSize(quint32 block_size)
  {
    return ((m_header.width + 3) >> 2) * ((m_header.height + 3) >> 2) * block_size;
  }

  char* m_image_data_buffer = nullptr;
  QByteArray m_image_data;
  DDSHeader m_header;
};
