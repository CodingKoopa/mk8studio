#ifndef GTX_H
#define GTX_H

#include <QByteArray>

#include "Common.h"
#include "FormatBase.h"

class GX2ImageBase : public FormatBase
{
public:
  GX2ImageBase() : m_header(nullptr) {}
  ~GX2ImageBase()
  {
    if (m_header)
      delete m_header;
  }
  enum Format
  {
    GX2_FMT_INVALID = 0x0,
    GX2_FMT_8 = 0x1,
    GX2_FMT_4_4 = 0x2,
    GX2_FMT_3_3_2 = 0x3,
    GX2_FMT_RESERVED_4 = 0x4,
    GX2_FMT_16 = 0x5,
    GX2_FMT_16_FLOAT = 0x6,
    GX2_FMT_8_8 = 0x7,
    GX2_FMT_5_6_5 = 0x8,
    GX2_FMT_6_5_5 = 0x9,
    GX2_FMT_1_5_5_5 = 0xA,
    GX2_FMT_4_4_4_4 = 0xB,
    GX2_FMT_5_5_5_1 = 0xC,
    GX2_FMT_32 = 0xD,
    GX2_FMT_32_FLOAT = 0xE,
    GX2_FMT_16_16 = 0xF,
    GX2_FMT_16_16_FLOAT = 0x10,
    GX2_FMT_8_24 = 0x11,
    GX2_FMT_8_24_FLOAT = 0x12,
    GX2_FMT_24_8 = 0x13,
    GX2_FMT_24_8_FLOAT = 0x14,
    GX2_FMT_10_11_11 = 0x15,
    GX2_FMT_10_11_11_FLOAT = 0x16,
    GX2_FMT_11_11_10 = 0x17,
    GX2_FMT_11_11_10_FLOAT = 0x18,
    GX2_FMT_2_10_10_10 = 0x19,
    GX2_FMT_8_8_8_8 = 0x1A,
    GX2_FMT_10_10_10_2 = 0x1B,
    GX2_FMT_X24_8_32_FLOAT = 0x1C,
    GX2_FMT_32_32 = 0x1D,
    GX2_FMT_32_32_FLOAT = 0x1E,
    GX2_FMT_16_16_16_16 = 0x1F,
    GX2_FMT_16_16_16_16_FLOAT = 0x20,
    GX2_FMT_RESERVED_33 = 0x21,
    GX2_FMT_32_32_32_32 = 0x22,
    GX2_FMT_32_32_32_32_FLOAT = 0x23,
    GX2_FMT_RESERVED_36 = 0x24,
    GX2_FMT_1 = 0x25,
    GX2_FMT_1_REVERSED = 0x26,
    GX2_FMT_GB_GR = 0x27,
    GX2_FMT_BG_RG = 0x28,
    GX2_FMT_32_AS_8 = 0x29,
    GX2_FMT_32_AS_8_8 = 0x2A,
    GX2_FMT_5_9_9_9_SHAREDEXP = 0x2B,
    GX2_FMT_8_8_8 = 0x2C,
    GX2_FMT_16_16_16 = 0x2D,
    GX2_FMT_16_16_16_FLOAT = 0x2E,
    GX2_FMT_32_32_32 = 0x2F,
    GX2_FMT_32_32_32_FLOAT = 0x30,
    GX2_FMT_BC1_UNORM = 0x31,
    GX2_FMT_BC2_UNORM = 0x32,
    GX2_FMT_BC3 = 0x33,
    GX2_FMT_BC4_UNORM = 0x34,
    GX2_FMT_BC5_UNORM = 0x35,
    GX2_FMT_BC6 = 0x36,
    GX2_FMT_BC7 = 0x37,
    GX2_FMT_32_AS_32_32_32_32 = 0x38,
    GX2_FMT_APC3 = 0x39,
    GX2_FMT_APC4 = 0x3A,
    GX2_FMT_APC5 = 0x3B,
    GX2_FMT_APC6 = 0x3C,
    GX2_FMT_APC7 = 0x3D,
    GX2_FMT_CTX1 = 0x3E,
    GX2_FMT_RESERVED_63 = 0x3F,
    GX2_FMT_BC4_SNORM = 0x234,
    GX2_FMT_BC5_SNORM = 0x235,
    GX2_FMT_BC1_SRGB = 0x431,
    GX2_FMT_BC2_SRGB = 0x432
  };

  enum TileMode
  {
    GX2_TILING_LINEAR_GENERAL = 0x0,
    GX2_TILING_LINEAR_ALIGNED = 0x1,
    GX2_TILING_1D_TILED_THIN1 = 0x2,
    GX2_TILING_1D_TILED_THICK = 0x3,
    GX2_TILING_2D_TILED_THIN1 = 0x4,
    GX2_TILING_2D_TILED_THIN2 = 0x5,
    GX2_TILING_2D_TILED_THIN4 = 0x6,
    GX2_TILING_2D_TILED_THICK = 0x7,
    GX2_TILING_2B_TILED_THIN1 = 0x8,
    GX2_TILING_2B_TILED_THIN2 = 0x9,
    GX2_TILING_2B_TILED_THIN4 = 0xA,
    GX2_TILING_2B_TILED_THICK = 0xB,
    GX2_TILING_3D_TILED_THIN1 = 0xC,
    GX2_TILING_3D_TILED_THICK = 0xD,
    GX2_TILING_3B_TILED_THIN1 = 0xE,
    GX2_TILING_3B_TILED_THICK = 0xF,
    GX2_TILING_LINEAR_SPECIAL = 0x10,
    GX2_TILING_2D_TILED_XTHICK = 0x10,
    GX2_TILING_3D_TILED_XTHICK = 0x11,
    GX2_TILING_POWER_SAVE = 0x12,
    GX2_TILING_COUNT = 0x13,
  };

  enum MicroTileType
  {
    GX2_MICRO_TILING_DISPLAYABLE = 0x0,
    GX2_MICRO_TILING_NON_DISPLAYABLE = 0x1,
    GX2_MICRO_TILING_DEPTH_SAMPLE_ORDER = 0x2,
    GX2_MICRO_TILING_THICK_TILING = 0x3,
  };

  struct ImageHeader
  {
    virtual ~ImageHeader() {}
    quint32 data_length;
    quint32 width;
    quint32 height;
    quint32 pitch;
    quint32 num_mips;
    quint32 aa_mode;
    Format format;
    TileMode tile_mode;
    quint32 swizzle;
    // TODO: Do something with this when deswizzling, set m_has_depth?
    quint32 depth;
  };

  ResultCode ReadImageFromData();

  // temporary
  void SetName(const QString& value);

protected:
  ImageHeader* m_header;
  QByteArray raw_image_data;

private:
  // Get the address of a pixel from a coordinate, within a macro tiled texture
  quint64 ComputeSurfaceAddrFromCoordMacroTiled(quint32 x, quint32 y, quint32 slice, quint32 sample,
                                                quint32 tileBase, quint32 compBits,
                                                quint32* pBitPosition);

  // Get the index of a pixel from a coordinate, within a micro tile
  quint32 ComputePixelIndexWithinMicroTile(quint32 x, quint32 y, quint32 z);

  // Computation of various things.
  void ComputeMicroTileType();
  void ComputeSurfaceThickness();
  void ComputeSurfaceRotationFromTileMode();
  void ComputeThickMicroTiling();
  void ComputeBankSwappedTileMode();

  quint32 ComputeMacroTileAspectRatio();

  quint32 ComputeSurfaceBankSwappedWidth(quint32 m_pitch, quint32* pSlicesPerTile);

  quint32 ComputePipeFromCoordWoRotation(quint32 x, quint32 y);

  quint32 ComputeBankFromCoordWoRotation(quint32 x, quint32 y);

  QByteArray deswizzled_image_data;

  // Constants
  quint32 m_pipes = 2;
  quint32 m_banks = 4;
  quint32 m_group_bit_count = 8;
  quint32 m_pipe_bit_count = 1;
  quint32 m_bank_bit_count = 2;
  quint32 m_split_size = 2048;
  quint32 m_swap_size = 256;
  quint32 m_row_size = 2048;
  quint32 m_pipe_interleave_bytes = 256;
  const quint32 m_micro_tile_width = 8;
  const quint32 m_micro_tile_height = 8;
  const quint32 m_num_micro_tile_pixels = m_micro_tile_width * m_micro_tile_height;
  const quint32 ThickTileThickness = 4;
  const quint32 XThickTileThickness = 8;
  const quint32 HtileCacheBits = 16384;

  // TODO: reorder these more logically
  quint32 m_bpp;
  quint32 m_num_samples;
  quint32 m_pipe_swizzle;
  quint32 m_bank_swizzle;
  MicroTileType m_micro_tile_type;
  quint64 m_slice_bytes;
  quint64 m_slice_offset;
  quint32 m_micro_tile_thickness;
  bool m_is_thick_macro_tiled;
  bool bankSwappedTileMode;
  bool m_has_depth;
  quint32 rotate;

  // Micro Tiling Info
  quint64 m_num_micro_tile_bits;
  quint64 m_num_micro_tile_bytes;
  quint64 m_bytes_per_sample;

  // Macro Tiling info
  quint64 m_macro_tile_pitch;
  quint64 m_macro_tile_height;
  quint64 m_macro_tiles_per_row;
  quint64 m_num_macro_tile_bytes;
};

#endif  // GTX_H
