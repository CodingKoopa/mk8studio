#pragma once

#include <QByteArray>

#include "Common.h"
#include "Formats/FormatBase.h"

class GX2ImageBase : public FormatBase
{
public:
  // Parameter passthrough.
  GX2ImageBase(std::shared_ptr<File> file, quint32 start_offset = 0, quint32 header_size = 0);

  // Common kinds of formats.
  enum class CommonFormat
  {
    Invalid = 0x0,
    BC1 = 0x31,
    BC4 = 0x34,
    BC5 = 0x35
  };

  // Information about a common format.
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

  // Information about a specific format.
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

  // Tile Mode enumerations just for convienience.
  enum class CommonTileMode
  {
    Linear,
    Micro,
    Macro
  };

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

  struct ImageHeaderBase
  {
    virtual ~ImageHeaderBase() {}
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

  ResultCode SetupInfo();

  ResultCode ReadImageFromData();
  ResultCode WriteDeswizzledImageToData();

  ResultCode ImportDDS(QString path);
  ResultCode ExportToDDS(QString path);

  static std::map<quint32, FormatInfo> GetFormatInfos();
  static std::map<quint32, TileModeInfo> GetTileModeInfos();

  const FormatInfo& GetFormatInfo() const;
  const TileModeInfo& GetTileModeInfo() const;

protected:
  ImageHeaderBase m_base_header;
  QByteArray m_raw_image_data;

private:
  ResultCode CopyImage(QByteArray* source, QByteArray* destination, bool swizzle);

  // Get the address of a pixel from a coordinate, within a macro tiled texture
  quint64 ComputeSurfaceAddrFromCoordMacroTiled(quint32 x, quint32 y, quint32 slice, quint32 sample,
                                                quint32 tile_base, quint32 compBits);

  // Get the index of a pixel from a coordinate, within a micro tile
  quint32 ComputePixelIndexWithinMicroTile(quint32 x, quint32 y, quint32 z);

  quint32 ComputeSurfaceBankSwappedWidth(quint32 m_pitch);

  // "information" is an uncountable word, but we need to differentiate between its singular and
  // plural forms.
  static const inline std::map<CommonFormat, CommonFormatInfo> m_common_format_infos{
      {CommonFormat::Invalid, {0, CommonFormatInfo::Use::None, false}},
      {CommonFormat::BC1, {64, CommonFormatInfo::Use::Texture, true}},
      {CommonFormat::BC4, {64, CommonFormatInfo::Use::Texture, true}},
      {CommonFormat::BC5, {128, CommonFormatInfo::Use::Texture, true}}};

  // Very incomplete list of formats.
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

  // Instead of having a CommonTileModeInfo struct, only keep track of the rotations because they
  // are the only shared feature.
  // clang-format off
  const std::map<CommonTileMode, quint32> m_common_tile_mode_rotations{
      {CommonTileMode::Linear,  0},
      {CommonTileMode::Micro,   NUM_PIPES*((NUM_BANKS >> 1) - 1)},
      {CommonTileMode::Macro,   1}};
  // clang-format on

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

  QByteArray m_deswizzled_image_data;

  enum class MicroTileType
  {
    Displayable = 0x0,
    NonDisplayable = 0x1,
    DepthSampleOrder = 0x2,
    ThickTiliing = 0x3,
  };

  // Micro Tiling Info
  static constexpr quint32 MICRO_TILE_WIDTH = 8;
  static constexpr quint32 MICRO_TILE_HEIGHT = 8;
  static constexpr quint32 NUM_MICRO_TILE_PIXELS = MICRO_TILE_WIDTH * MICRO_TILE_HEIGHT;

  static constexpr quint32 NUM_BANKS = 4;
  static constexpr quint32 NUM_BANK_BITS = 2;
  static constexpr quint32 NUM_PIPES = 2;
  static constexpr quint32 NUM_PIPE_BITS = 1;
  static constexpr quint32 NUM_LOW_OFFSET_BITS = 8;

  static constexpr quint32 SPLIT_SIZE = 2048;
  static constexpr quint32 SWAP_SIZE = 256;
  static constexpr quint32 ROW_SIZE = 2048;
  static constexpr quint32 PIPE_INTERLEAVE_BYTES = 256;

  // Common format of the texture.
  CommonFormatInfo m_common_format_info;

  // Specific format of the texture.
  FormatInfo m_format_info;

  // Specific tile mode of the texture.
  TileModeInfo m_tile_mode_info;

  // Common rotation of the texture.
  quint32 m_common_tile_mode_rotation;

  quint32 m_element_size;
  quint32 m_num_samples;

  MicroTileType m_micro_tile_type;
  quint64 m_num_micro_tile_bits;
  quint64 m_num_micro_tile_bytes;
  quint64 m_bytes_per_sample;

  quint64 m_macro_tile_pitch;
  quint64 m_macro_tile_height;
  quint64 m_macro_tiles_per_row;
  quint64 m_num_macro_tile_bytes;

  quint64 m_num_slice_bytes;

  quint32 m_pipe_swizzle;
  quint32 m_bank_swizzle;
};
