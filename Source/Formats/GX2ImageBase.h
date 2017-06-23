#ifndef GTX_H
#define GTX_H

#include <QByteArray>
#include <QList>

#include "Common.h"
#include "FormatBase.h"

class GX2ImageBase : public FormatBase
{
public:
  GX2ImageBase() : m_base_header(nullptr) {}

  // Format enumerations just for convienience.
  enum class Format
  {
    Invalid = 0x0,
    BC1 = 0x31,
    BC4 = 0x34,
    BC5 = 0x35
  };

  // Info that's consistent across any texture with the same format.
  struct SharedFormatInfo
  {
    Format format;
    quint32 bpp;

    enum class Use
    {
      // Not a real use.
      None = 0,
      Texture = 1 << 0,
      ColorBuffer = 1 << 1,
      DepthBuffer = 1 << 2,
      ScanBuffer = 1 << 3
    } use;

    bool compressed;
  };

  struct FormatInfo
  {
    // This is used in both info structs, but making a base class would make them become non
    // aggregates. C++17 will allow public inherited base classes in aggregates.
    quint32 id;
    QString name;

    Format format;

    enum class Type
    {
      // TODO: Values for these
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
  enum class TileMode
  {
    Linear,
    Micro,
    Macro
  };

  struct SharedTileModeInfo
  {
    TileMode mode;

    quint32 rotation;
  };

  struct TileModeInfo
  {
    quint32 id;
    QString name;

    TileMode mode;

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

  void SetupInfoStructs();

  ResultCode ReadImageFromData();
  ResultCode WriteDeswizzledImageToData();

  ResultCode ImportDDS(QString path);
  ResultCode ExportToDDS(QString path);

  QList<FormatInfo> GetFormatInfoList() { return m_format_info_list; }
  QList<TileModeInfo> GetTileModeInfoList() { return m_tile_mode_info_list; }

  FormatInfo GetFormatInfo() { return m_format_info; }
  quint32 GetFormatInfoIndex() { return m_format_info_index; }
  TileModeInfo GetTileModeInfo() { return m_tile_mode_info; }

protected:
  ImageHeaderBase* m_base_header;
  QByteArray* m_raw_image_data;

private:
  ResultCode CopyImage(QByteArray* source, QByteArray*& destination, bool swizzle);

  // Get the address of a pixel from a coordinate, within a macro tiled texture
  quint64 ComputeSurfaceAddrFromCoordMacroTiled(quint32 x, quint32 y, quint32 slice, quint32 sample,
                                                quint32 tile_base, quint32 compBits);

  // Get the index of a pixel from a coordinate, within a micro tile
  quint32 ComputePixelIndexWithinMicroTile(quint32 x, quint32 y, quint32 z);

  quint32 ComputeSurfaceBankSwappedWidth(quint32 m_pitch);

  QByteArray* m_deswizzled_image_data;

  // Constants
  bool m_optimal_bank_swap = 1;
  quint32 m_group_bit_count = 8;
  quint32 m_bank_bit_count = 2;
  quint32 m_pipe_bit_count = 1;
  quint32 m_split_size = 2048;
  quint32 m_swap_size = 256;
  quint32 m_row_size = 2048;
  quint32 m_pipe_interleave_bytes = 256;
  const quint32 m_num_pipes = 2;
  const quint32 m_num_banks = 4;
  const quint32 m_micro_tile_width = 8;
  const quint32 m_micro_tile_height = 8;
  const quint32 m_num_micro_tile_pixels = m_micro_tile_width * m_micro_tile_height;

  const quint32 m_tile_mode_2_rotation = m_num_pipes * ((m_num_banks >> 1) - 1);
  const quint32 m_tile_mode_3_rotation = (m_num_pipes >= 4) ? ((m_num_banks >> 1) - 1) : 1;

  enum class MicroTileType
  {
    Displayable = 0x0,
    NonDisplayable = 0x1,
    DepthSampleOrder = 0x2,
    ThickTiliing = 0x3,
  };

  const QList<SharedFormatInfo> m_shared_format_info_list{
      {Format::Invalid, 0, SharedFormatInfo::Use::None, false},
      {Format::BC1, 64, SharedFormatInfo::Use::Texture, true},
      {Format::BC4, 64, SharedFormatInfo::Use::Texture, true},
      {Format::BC5, 128, SharedFormatInfo::Use::Texture, true}};

  const FormatInfo zerp {0x0, "GX2_SURFACE_FORMAT_INVALID", Format::Invalid, FormatInfo::Type::Invalid};

  // Very incomplete list of formats.
  const QList<FormatInfo> m_format_info_list{
      // Invalid
      {0x0, "GX2_SURFACE_FORMAT_INVALID", Format::Invalid, FormatInfo::Type::Invalid},
      // Unsigned Normals
      {0x31, "GX2_SURFACE_FORMAT_UNORM_BC1", Format::BC1, FormatInfo::Type::UNorm},
      {0x34, "GX2_SURFACE_FORMAT_UNORM_BC4", Format::BC4, FormatInfo::Type::UNorm},
      {0x35, "GX2_SURFACE_FORMAT_UNORM_BC5", Format::BC4, FormatInfo::Type::UNorm},
      // Signed Normals
      {0x234, "GX2_SURFACE_FORMAT_SNORM_BC4", Format::BC4, FormatInfo::Type::SNorm},
      {0x235, "GX2_SURFACE_FORMAT_SNORM_BC5", Format::BC5, FormatInfo::Type::SNorm},
      // SRGB
      {0x431, "GX2_SURFACE_FORMAT_SRGB_BC1", Format::BC1, FormatInfo::Type::SRGB}};

  const QList<SharedTileModeInfo> m_shared_tile_mode_info_list{
      // TODO: add INVALID entry?
      {TileMode::Linear, 0},
      {TileMode::Micro, m_tile_mode_2_rotation},
      {TileMode::Macro, m_tile_mode_3_rotation}};

  const QList<TileModeInfo> m_tile_mode_info_list{
      // Linear Tiled
      {0x0, "GX2_TILE_MODE_DEFAULT", TileMode::Linear, TileModeInfo::Thickness::Thin, 0, false},
      {0x1, "GX2_TILE_MODE_LINEAR_ALIGNED", TileMode::Linear, TileModeInfo::Thickness::Thin, 0,
       false},
      // Micro Tiled
      {0x2, "GX2_TILE_MODE_1D_TILED_THIN1", TileMode::Micro, TileModeInfo::Thickness::Thin, 0,
       false},
      {0x3, "GX2_TILE_MODE_1D_TILED_THICK", TileMode::Micro, TileModeInfo::Thickness::Thick, 0,
       false},
      // Macro Tiled
      {0x4, "GX2_TILE_MODE_2D_TILED_THIN1", TileMode::Macro, TileModeInfo::Thickness::Thin, 1,
       false},
      {0x5, "GX2_TILE_MODE_2D_TILED_THIN2", TileMode::Macro, TileModeInfo::Thickness::Thin, 2,
       false},
      {0x6, "GX2_TILE_MODE_2D_TILED_THIN4", TileMode::Macro, TileModeInfo::Thickness::Thin, 4,
       false},
      {0x7, "GX2_TILE_MODE_2D_TILED_THICK", TileMode::Macro, TileModeInfo::Thickness::Thick, 1,
       false},
      {0x8, "GX2_TILE_MODE_2B_TILED_THIN1", TileMode::Macro, TileModeInfo::Thickness::Thin, 1,
       false},
      {0x9, "GX2_TILE_MODE_2B_TILED_THIN2", TileMode::Macro, TileModeInfo::Thickness::Thin, 2,
       false},
      {0xA, "GX2_TILE_MODE_2B_TILED_THIN4", TileMode::Macro, TileModeInfo::Thickness::Thin, 4,
       false},
      {0xB, "GX2_TILE_MODE_2B_TILED_THICK", TileMode::Macro, TileModeInfo::Thickness::Thick, 1,
       false},
      {0xC, "GX2_TILE_MODE_3D_TILED_THIN1", TileMode::Macro, TileModeInfo::Thickness::Thin, 1,
       false},
      {0xD, "GX2_TILE_MODE_3D_TILED_THICK", TileMode::Macro, TileModeInfo::Thickness::Thick, 1,
       false},
      {0xE, "GX2_TILE_MODE_3B_TILED_THIN1", TileMode::Macro, TileModeInfo::Thickness::Thin, 1,
       false},
      {0xF, "GX2_TILE_MODE_3B_TILED_THICK", TileMode::Macro, TileModeInfo::Thickness::Thick, 1,
       false}};
  // LINEAR_SPECIAL is also a thing, along with XTHICK, POWER_SAVE, and COUNT, which aren't included
  // because Decaf does not read them.

  // TODO: reorder these more logically
  quint32 m_num_samples;
  quint32 m_pipe_swizzle;
  quint32 m_bank_swizzle;
  MicroTileType m_micro_tile_type;
  quint64 m_num_slice_bytes;
  quint64 m_slice_offset;
  bool m_has_depth;

  // Micro Tiling Info
  quint64 m_num_micro_tile_bits;
  quint64 m_num_micro_tile_bytes;
  quint64 m_bytes_per_sample;

  // Macro Tiling info
  quint64 m_macro_tile_pitch;
  quint64 m_macro_tile_height;
  quint64 m_macro_tiles_per_row;
  quint64 m_num_macro_tile_bytes;

  FormatInfo m_format_info;
  // The index of the current format, in the list.
  quint32 m_format_info_index;
  SharedFormatInfo m_shared_format_info;
  TileModeInfo m_tile_mode_info;
  SharedTileModeInfo m_shared_tile_mode_info;
};

#endif  // GTX_H
