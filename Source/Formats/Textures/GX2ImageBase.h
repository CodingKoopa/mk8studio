#pragma once

#include <QByteArray>

#include "Common.h"
#include "Formats/FormatBase.h"

/// Abstracts common aspects of Wii U textures. For more technical information, see the GX2 page.
///
/// Currently, the implementations of the methods here are heavily annotated in lieu of proper
/// documentation. When a proper write-up of the tiling and swizzling algorithms is made, the
/// comments will be removed.
///
/// @todo GX2TextureBase might be a more appropriate name for this class.
class GX2ImageBase : public FormatBase
{
public:
  /// Initializes a new instance of the GX2ImageBase class. This is just a passthrough for all of
  /// the parameters.
  ///
  /// @param  file          Shared pointer to the file to read from.
  /// @param  start_offset  The offset to start reading at, where the FVTX structure starts.
  /// @param  header_size   The size of the format header.
  GX2ImageBase(std::shared_ptr<File> file, quint32 start_offset = 0, quint32 header_size = 0);

  /// The common, shared formats. There are a lot of GX2 formats, but they can be grouped together
  /// in what kind of graphics compression format they use, like BC1 compression.
  enum class CommonFormat
  {
    Invalid = 0x0,
    BC1 = 0x31,
    BC4 = 0x34,
    BC5 = 0x35
  };

  /// @brief Contains info about a common format that can be collected from the kind of specific
  /// format.
  ///
  /// @todo The fields here can be documented.
  struct CommonFormatInfo
  {
    quint32 bpp;
    enum class Use
    {
      // Not a real use.
      None,
      Texture,
      ColorBuffer,
      DepthBuffer,
      ScanBuffer
    } use;
    bool compressed;
  };

  /// @brief Contains info about a specific format that can be collected from the ID.
  ///
  /// @todo The fields here can be documented.
  struct FormatInfo
  {
    CommonFormat common_format;

    // Use Decaf's name for the format.
    QString name;
    enum class Type
    {
      Invalid,
      UNorm,
      UInt,
      SNorm,
      SInt,
      SRGB,
      Float
    } type;
  };

  /// The common, shared color tiling modes. There are a lot of GX2 tiling modes, but they can be
  /// grouped together in what kind of tiling they use. The shorthand conventions
  /// used to refer to each mode follows the format of xD, where x is a number representing where
  /// the tile falls in the hierarchy. For example, a tile of format 2D is larger than, and contains
  /// tiles of format 1D. These names have no relation to dimensions, e.g. 3D means a tile with a
  /// relative size of 3, and not 3-dimensional.
  ///
  /// @todo The fields here can be documented.
  enum class CommonTileMode
  {
    Linear,
    /// Micro tiles, or 1D tiles, group pixels into relatively simple tiles. When addressing a
    /// coordinate of a micro tile, GX2ImageBase::GetPixelIndexMicroTiled() will interleave the X,
    /// Y, and (if applicable) Z coordinates to find the index of a pixel in a micro tile.
    Micro,
    Macro
  };

  /// @brief Contains info about a specific tile mode that can be collected from the ID.
  ///
  /// @todo The fields here can be documented.
  struct TileModeInfo
  {
    CommonTileMode mode;

    QString name;
    enum class Thickness
    {
      Thin = 1,
      Thick = 4
    } thickness;
    quint32 aspect_ratio;
    bool swap_banks;
  };

  /// @brief Represents the common parts of the %FTEX and %BFLIM headers.
  ///
  /// @todo The fields here can be documented.
  struct HeaderBase
  {
    quint32 data_length;
    quint32 mipmap_length;
    quint32 width;
    quint32 height;
    quint32 pitch;

    quint32 format;
    quint32 tile_mode;

    quint32 aa_mode;
    quint32 num_mips;
    // This currently isn't used, should only be needed for tile mode 3 textures.
    quint32 depth;

    quint32 swizzle;
  };

  /// Reads the raw GX2 image data, and deswizzles it.
  ///
  /// @return The success of the reading.
  ResultCode ReadImageFromData();

  /// Reads the deswizzled image data from a %DDS file, swizzles it, and copies it to the GX2 image
  /// data.
  ///
  /// @param  path  The path to write the new file to.
  ///
  /// @return The success of the writing.
  ResultCode ImportDDS(QString path);
  /// Writes the deswizzled image data to a %DDS file.
  ///
  /// @param  path  The path to write the new file to.
  ///
  /// @return The success of the writing.
  ResultCode ExportToDDS(QString path);

  /// Gets the format info reference.
  ///
  /// @return The format info reference.
  static std::map<quint32, FormatInfo> GetFormatInfos();
  /// Gets the format info reference.
  ///
  /// @return The format info reference.
  static std::map<quint32, TileModeInfo> GetTileModeInfos();

  /// Gets the parsed format info.
  ///
  /// @return The parsed format info.
  const FormatInfo& GetFormatInfo() const;
  /// Gets the parsed tile mode info.
  ///
  /// @return The parsed tile mode info.
  const TileModeInfo& GetTileModeInfo() const;

protected:
  /// Sets up the info structures, from the format and tile mode.
  ///
  /// @return The success, if an info entry exsists for every format/tile mode.
  ResultCode SetupInfo();

  /// The common GX2 header parsed from the file.
  HeaderBase m_base_header;
  /// The raw, swizzled image data parsed from the fle.
  QByteArray m_raw_image_data;

private:
  /// Copies image data, either unswizzling or swizzling it.
  ///
  /// @param  source      The source image data to copy from.
  /// @param  destination The destination image data to copy to.
  /// @param  swizzle     Whether to swizzle the texture while copying or not.
  ///
  /// @return The success of copying.
  ///
  /// @todo A proper error should be thrown if this method attmpts to read or write outside of the
  /// image boundaries.
  ResultCode CopyImage(QByteArray* source, QByteArray* destination, bool swizzle);

  /// Gets the address of a pixel from a coordinate, within a macro tiled texture.
  ///
  /// @param  x         The X coordinate of the pixel.
  /// @param  y         The Y coordinate of the pixel.
  /// @param  slice     Unknown, Z coordinate? Used for 3D textures.
  /// @param  sample    Which sample. Used for textures with anti-aliasing.
  /// @param  tile_base Unknown. Used for 3D textures.
  /// @param  comp_bits Unknown. Used for 3D textures.
  ///
  /// @todo 3D texture support is present from Decaf, but untested and undocumented.
  quint64 GetPixelOffsetMacroTiled(quint32 x, quint32 y, quint32 slice, quint32 sample,
                                   quint32 tile_base, quint32 comp_bits);

  /// Gets the index of a pixel from a coordinate, within a micro tile.
  ///
  /// @param  x The X coordinate of the pixel.
  /// @param  y The Y coordinate of the pixel.
  /// @param  z The Z coordinate of the pixel.
  ///
  /// @return The index of the pixel relative to the start of the micro tile. Also known as the
  /// pixel number. NOT the pixel offset, that's different.
  quint32 GetPixelIndexMicroTiled(quint32 x, quint32 y, quint32 z);

  /// Get the width of the texture, with bank swapping in mind.
  quint32 ComputeSurfaceBankSwappedWidth(quint32 pitch);

  /// The number of pixels in one row of a micro tile.
  static constexpr quint32 MICRO_TILE_WIDTH = 8;
  /// The number of pixels in one column of a micro tile.
  static constexpr quint32 MICRO_TILE_HEIGHT = 8;
  /// The number of pixels in one micro tile.
  static constexpr quint32 NUM_MICRO_TILE_PIXELS = MICRO_TILE_WIDTH * MICRO_TILE_HEIGHT;

  /// The number of banks, the number of RAM chips.
  static constexpr quint32 NUM_BANKS = 4;
  /// The number of bits of a pixel address that the bank occupies.
  static constexpr quint32 NUM_BANK_BITS = 2;
  /// The number of pipes, the number of RAM channels in one chip.
  static constexpr quint32 NUM_PIPES = 2;
  /// The number of bits of a pixel address that the pipe occupies.
  static constexpr quint32 NUM_PIPE_BITS = 1;
  /// The number of bits of a pixel address that the lower part occupies, before the pipe and bank.
  static constexpr quint32 NUM_LOW_OFFSET_BITS = 8;

  /// Unknown.
  ///
  /// @todo Document this.
  static constexpr quint32 SPLIT_SIZE = 2048;
  /// Unknown.
  ///
  /// @todo Document this.
  static constexpr quint32 SWAP_SIZE = 256;
  /// Unknown.
  ///
  /// @todo Document this.
  static constexpr quint32 ROW_SIZE = 2048;
  /// Unknown.
  ///
  /// @todo Document this.
  static constexpr quint32 PIPE_INTERLEAVE_BYTES = 256;

  /// Reference for the info about each common format.
  static const inline std::map<CommonFormat, CommonFormatInfo> m_common_format_infos{
      {CommonFormat::Invalid, {0, CommonFormatInfo::Use::None, false}},
      {CommonFormat::BC1, {64, CommonFormatInfo::Use::Texture, true}},
      {CommonFormat::BC4, {64, CommonFormatInfo::Use::Texture, true}},
      {CommonFormat::BC5, {128, CommonFormatInfo::Use::Texture, true}}};

  /// Reference for the info about each specific format.
  ///
  /// @todo Add the rest.
  // clang-format off
  static const inline std::map<quint32, FormatInfo> m_format_infos{
      // Invalid
      {0x0,     {CommonFormat::Invalid, "GX2_SURFACE_FORMAT_INVALID",   FormatInfo::Type::Invalid}},
      // Unsigned Normals
      {0x31,    {CommonFormat::BC1,     "GX2_SURFACE_FORMAT_UNORM_BC1", FormatInfo::Type::UNorm}},
      {0x34,    {CommonFormat::BC4,     "GX2_SURFACE_FORMAT_UNORM_BC4", FormatInfo::Type::UNorm}},
      {0x35,    {CommonFormat::BC4,     "GX2_SURFACE_FORMAT_UNORM_BC5", FormatInfo::Type::UNorm}},
      // Signed Normals
      {0x234,   {CommonFormat::BC4,     "GX2_SURFACE_FORMAT_SNORM_BC4", FormatInfo::Type::SNorm}},
      {0x235,   {CommonFormat::BC5,     "GX2_SURFACE_FORMAT_SNORM_BC5", FormatInfo::Type::SNorm}},
      // SRGB
      {0x431,   {CommonFormat::BC1,     "GX2_SURFACE_FORMAT_SRGB_BC1",  FormatInfo::Type::SRGB}}};
  // clang-format on

  /// Reference for the rotations of each common tile mode. Note the absence of a CommonTileModeInfo
  /// struct. Here, the only thing in common is the rotation, so there's no need for a dedicated
  /// type.
  // clang-format off
  static const inline std::map<CommonTileMode, quint32> m_common_tile_mode_rotations{
      {CommonTileMode::Linear,  0},
      {CommonTileMode::Micro,   NUM_PIPES*((NUM_BANKS >> 1) - 1)},
      {CommonTileMode::Macro,   1}};
  // clang-format on

  /// Reference for the info about each speciic tile mode.
  // clang-format off
  static const inline std::map<quint32, TileModeInfo> m_tile_mode_infos{
      // Linear Tiled
      {0x0,     {CommonTileMode::Linear,    "GX2_TILE_MODE_DEFAULT",        TileModeInfo::Thickness::Thin,  0, false}},
      {0x1,     {CommonTileMode::Linear,    "GX2_TILE_MODE_LINEAR_ALIGNED", TileModeInfo::Thickness::Thin,  0, false}},
      // Micro Tiled
      {0x2,     {CommonTileMode::Micro,     "GX2_TILE_MODE_1D_TILED_THIN1", TileModeInfo::Thickness::Thin,  0, false}},
      {0x3,     {CommonTileMode::Micro,     "GX2_TILE_MODE_1D_TILED_THICK", TileModeInfo::Thickness::Thick, 0, false}},
      // Macro Tiled
      {0x4,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2D_TILED_THIN1", TileModeInfo::Thickness::Thin,  1, false}},
      {0x5,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2D_TILED_THIN2", TileModeInfo::Thickness::Thin,  2, false}},
      {0x6,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2D_TILED_THIN4", TileModeInfo::Thickness::Thin,  4, false}},
      {0x7,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2D_TILED_THICK", TileModeInfo::Thickness::Thick, 1, false}},
      {0x9,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2B_TILED_THIN2", TileModeInfo::Thickness::Thin,  2, true}},
      {0x8,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2B_TILED_THIN1", TileModeInfo::Thickness::Thin,  1, true}},
      {0xA,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2B_TILED_THIN4", TileModeInfo::Thickness::Thin,  4, true}},
      {0xB,     {CommonTileMode::Macro,     "GX2_TILE_MODE_2B_TILED_THICK", TileModeInfo::Thickness::Thick, 1, true}},
      {0xC,     {CommonTileMode::Macro,     "GX2_TILE_MODE_3D_TILED_THIN1", TileModeInfo::Thickness::Thin,  1, false}},
      {0xD,     {CommonTileMode::Macro,     "GX2_TILE_MODE_3D_TILED_THICK", TileModeInfo::Thickness::Thick, 1, false}},
      {0xE,     {CommonTileMode::Macro,     "GX2_TILE_MODE_3B_TILED_THIN1", TileModeInfo::Thickness::Thin,  1, true}},
      {0xF,     {CommonTileMode::Macro,     "GX2_TILE_MODE_3B_TILED_THICK", TileModeInfo::Thickness::Thick, 1, true}}};
  // clang-format on

  /// The deswizzled image data parsed from the raw image data.
  QByteArray m_deswizzled_image_data;

  /// The common format of the texture.
  CommonFormatInfo m_common_format_info;

  /// The specific format of the texture.
  FormatInfo m_format_info;

  /// The common rotation of the texture.
  quint32 m_common_tile_mode_rotation;

  /// The specific tile mode of the texture.
  TileModeInfo m_tile_mode_info;

  /// The size of an element. When talking about textures, elements refer to the smallest unit
  /// currently accessable of the texture. In an uncompressed texture, this is often just an RGBA
  /// value, a 1:1 representation of a given pixel. In a compressed texture, this is a whole
  /// self-contained tile, containing the dictionary of color values and then the table of colers.
  quint32 m_element_size;

  /// The number of samples in the texture. Tiles with sampling are self-contained. For example,
  /// this is what an excerpt from a micro tiled texture with 2 samples might look like:
  /// - Micro Tile 1
  ///  - Sample 1
  ///  - Sample 2
  /// - Micro Tile 2
  ///  - Sample 1
  ///  - Sample 2
  ///
  /// @todo How does this work for textures with linear tiling?
  quint32 m_num_samples;

  /// The types of micro tiles.
  ///
  /// @todo The fields here can be documented.
  enum class MicroTileType
  {
    Displayable = 0x0,
    NonDisplayable = 0x1,
    DepthSampleOrder = 0x2,
    ThickTiliing = 0x3,
  } m_micro_tile_type;

  /// The number of bits in one micro tile of the texture.
  quint64 m_num_micro_tile_bits;
  /// The number of bytes in one micro tile of the texture.
  quint64 m_num_micro_tile_bytes;
  /// The number of bytes in one sample of the texture.
  quint64 m_bytes_per_sample;

  /// The number of bytes between the start of one row of pixels and the next, of a macro tile.
  quint64 m_macro_tile_pitch;
  /// The number of pixels in one column of a macro tile.
  quint64 m_macro_tile_height;
  /// The number of macro tiles in one row of the texture.
  quint64 m_macro_tiles_per_row;
  /// The number of bytes in one macro tile of the texture
  quint64 m_num_macro_tile_bytes;

  /// Unknown.
  ///
  /// @todo Document this.
  quint64 m_num_slice_bytes;

  /// The bit of the pixel address containing the target pipe.
  quint32 m_pipe_swizzle;
  /// The bits of the pixel address containing the target bank.
  quint32 m_bank_swizzle;
};
