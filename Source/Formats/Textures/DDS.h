#pragma once

#include <QByteArray>

#include "Common.h"
#include "Formats/FormatBase.h"
#include "Formats/Textures/GX2ImageBase.h"

/// @brief Represents a <b>D</b>irect<b>D</b>raw <b>S</b>urface (%DDS) textire.
///
/// %DDS is Microsoft's format for storing a texture for usage in a 3D space, and its mipmaps. It's
/// useful to have as an option because unswizzled GX2 raw image data can be copied to a DDS and be
/// valid, as the compression is the same.
class DDS : public FormatBase
{
public:
  /// Initializes a new instance of the DDS class. This has to be marked as default or else the
  /// code won't compile, not sure why.
  DDS() = default;
  /// Initializes a new instance of the DDS class from an existing copy. Deleted so that the image
  /// data buffer is not double deleted.
  DDS(const DDS&) = delete;
  /// Assigns this instance of the DDS class to a new one. Deleted so that the image data buffer
  /// is not double deleted.
  DDS& operator=(const DDS&) = delete;
  /// Uninitializes an instance of the DDS class, deleting the image data buffer.
  ~DDS();

  /// @brief Represents the pixel format structure of the %DDS header.
  ///
  /// @todo The fields here can be documented.
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

  /// @brief Represents the %DDS header.
  ///
  /// @todo The fields here can be documented.
  struct Header
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

  /// Reads a DDS file at the path specified by FormatBase::m_path, parses the header into a Header,
  /// and reads the image data.
  ///
  /// @return   The success of the reading.
  ///
  /// @todo Handle the path not being set, or just accept the path as a parameter, because using
  /// FormatBase::m_path for both the reading and writing path is clunky.
  /// @todo Verify the number of bytes read.
  /// @todo Calculate the pitch, if necessary.
  ResultCode ReadFile();
  /// Writes the Header and image data to a DDS file to the path specified by FormatBase::m_path.
  ///
  /// @param    width               The width of the texture.
  /// @param    height              The height of the texture.
  /// @param    depth               The depth of the texture.
  /// @param    num_mips            The number of mipmaps in the texture.
  /// @param    element_size        The size of one element. See GX2ImageBase::m_element_size.
  /// @param    format_info         The specific format info of the texture.
  /// @param    common_format_info  The common format info of the texture.
  ///
  /// @todo Verify the number of bytes written.
  /// @todo Add uncompressed file support.
  /// @todo Add mipmap support.
  int WriteFile(quint32 width, quint32 height, quint32 depth, quint32 num_mips,
                quint32 element_size, GX2ImageBase::FormatInfo format_info,
                GX2ImageBase::CommonFormatInfo common_format_info);

  /// Gets the current image data.
  ///
  /// @return   The image data.
  const QByteArray& GetImageData() const;
  /// Sets the current image data.
  ///
  /// @param    The image data.
  void SetImageData(const QByteArray& image_data);

private:
  /// The possible image flags that can be used in the header.
  ///
  /// @todo The fields here can be documented.
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

  /// The possible complexity flags that can be used in the header.
  ///
  /// @todo The fields here can be documented.
  enum class ComplexityFlag
  {
    Complex = 0x8,
    Texture = 0x1000,
    HasMipMaps = 0x400000
  };

  /// The possible pixel flags that can be used in the pixel format.
  ///
  /// @todo The fields here can be documented.
  enum class PixelFlag
  {
    HasAlpha = 0x1,
    HasUncompressedAlpha = 0x2,
    IsCompressed = 0x4,
    IsUncompressedRGB = 0x40,
    IsUncompressedYUV = 0x200,
    HasLuminance = 0x20000,
  };

  /// Gets the linear size of a compressed texture. Not used for uncompressed textures.
  constexpr quint32 CalculateLinearSize(quint32 block_size)
  {
    return ((m_header.width + 3) >> 2) * ((m_header.height + 3) >> 2) * block_size;
  }

  /// The current %DDS header.
  Header m_header;

  /// The current, primarily used image data.
  QByteArray m_image_data;
  /// The image data buffer, temporarily used when moving image data to and from a %DDS file.
  char* m_image_data_buffer = nullptr;
};
