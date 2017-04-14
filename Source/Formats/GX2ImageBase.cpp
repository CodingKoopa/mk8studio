#include "GX2ImageBase.h"

#include "DDS.h"

ResultCode GX2ImageBase::ReadImageFromData()
{
  ComputeSurfaceThickness();
  ComputeSurfaceRotationFromTileMode();
  ComputeThickMicroTiling();
  ComputeBankSwappedTileMode();

  quint32 width = m_header->width;
  quint32 height = m_header->height;

  if (m_header->aa_mode == 0)
    m_num_samples = 1;
  else
    m_num_samples = m_header->aa_mode;
  m_pipe_swizzle = GET_BIT(m_header->swizzle, 8);
  // TODO: GET_BITS?
  m_bank_swizzle = (m_header->swizzle >> 9) & 3;

  switch (m_header->format)
  {
  case GX2_FMT_BC1_UNORM:
  case GX2_FMT_BC1_SRGB:
  case GX2_FMT_BC4_UNORM:
  case GX2_FMT_BC4_SNORM:
  {
    m_bpp = 0x40;
    width /= 4;
    height /= 4;
    break;
  }
  case GX2_FMT_BC5_UNORM:
  case GX2_FMT_BC5_SNORM:
  {
    m_bpp = 0x80;
    width /= 4;
    height /= 4;
    break;
  }
  default:
    return RESULT_UNSUPPORTED_FILE_FORMAT;
  }

  // Setup variables that are constant throughout the texture.
  switch (m_header->tile_mode)
  {
  // Macro Tiled
  case GX2_TILING_2D_TILED_THIN1:
  case GX2_TILING_2D_TILED_THIN2:
  case GX2_TILING_2D_TILED_THIN4:
  case GX2_TILING_2D_TILED_THICK:
  case GX2_TILING_2B_TILED_THIN1:
  case GX2_TILING_2B_TILED_THIN2:
  case GX2_TILING_2B_TILED_THIN4:
  case GX2_TILING_2B_TILED_THICK:
    m_macro_tile_pitch = 8 * m_banks;
    m_macro_tile_height = 8 * m_pipes;

    if (m_header->tile_mode == GX2_TILING_2D_TILED_THIN2 ||
        m_header->tile_mode == GX2_TILING_2B_TILED_THIN2)
    {
      m_macro_tile_pitch /= 2;
      m_macro_tile_height *= 2;
    }

    if (m_header->tile_mode == GX2_TILING_2D_TILED_THIN4 ||
        m_header->tile_mode == GX2_TILING_2B_TILED_THIN4)
    {
      m_macro_tile_pitch /= 4;
      m_macro_tile_height *= 4;
    }
    m_macro_tiles_per_row = m_header->pitch / m_macro_tile_pitch;

    // Number of samples in one macro tile *
    // Thickness of one micro tile =
    // Number to multiply with to account for sampling and thickness.

    // Height of one macro tile *
    // Pitch of one macro tile =
    // Number of pixels in each macro tile.

    // Number of pixels in one macro tile *
    // Number of bits in one pixel =
    // Number of bits in one macro tile.

    // Number of bits in one macro tile *
    // Number to multiply with to account for sampling and thickness =
    // Number of bits in one macro tile including all samples.

    // Number of bits in one macro tile including all samples -> Bytes =
    // Number of bytes in one macro tile.
    m_num_macro_tile_bytes = BITS_TO_BYTES(m_num_samples * m_micro_tile_thickness * m_bpp *
                                           m_macro_tile_height * m_macro_tile_pitch);

    //
    m_slice_bytes = BITS_TO_BYTES(m_header->pitch * m_header->height * m_micro_tile_thickness *
                                  m_bpp * m_num_samples);

  // Micro Tiled
  case GX2_TILING_1D_TILED_THIN1:
  case GX2_TILING_1D_TILED_THICK:
    if (m_has_depth)
      m_micro_tile_type = GX2_MICRO_TILING_DISPLAYABLE;
    else
      m_micro_tile_type = GX2_MICRO_TILING_DISPLAYABLE;

    // Number of pixels in one micro tile *
    // Thickness of one micro tile =
    // Number of pixels in one micro tile with thickness taken into account.

    // Recalculated number of pixels in one micro tile *
    // Number of bits in one pixel =
    // Number of bits in one sample of a micro tile.

    // Number of bits in one sample of a micro tile *
    // Number of samples in one micro tile =
    // Number of bits in one micro tile including all samples.
    m_num_micro_tile_bits =
        m_num_micro_tile_pixels * m_micro_tile_thickness * m_bpp * m_num_samples;

    // Number of bits in one micro tile with all samples /
    // Number of bits in a byte =
    // Number of bytes in one micro tile with all samples.
    // TODO: lambda for conversion to bits or something?
    m_num_micro_tile_bytes = m_num_micro_tile_bits / 8;

    // Number of bytes in one micro tile with all samples /
    // Number of samples in each micro tile =
    // Number of bytes in one sample of a micro tile.
    m_bytes_per_sample = m_num_micro_tile_bytes / m_num_samples;
    break;

  case GX2_TILING_LINEAR_GENERAL:
  case GX2_TILING_LINEAR_ALIGNED:
  default:
    return RESULT_UNSUPPORTED_FILE_FORMAT;
  }

  deswizzled_image_data.resize(m_header->data_length);
  deswizzled_image_data.fill(0);

  for (quint32 y = 0; y < height; ++y)
  {
    for (quint32 x = 0; x < width; ++x)
    {
      qint32 original_offset;
      // Get the offset of the pixel at the current coordinate.
      switch (m_header->tile_mode)
      {
      // Macro Tiled
      case GX2_TILING_2D_TILED_THIN1:
      case GX2_TILING_2D_TILED_THIN2:
      case GX2_TILING_2D_TILED_THIN4:
      case GX2_TILING_2D_TILED_THICK:
      case GX2_TILING_2B_TILED_THIN1:
      case GX2_TILING_2B_TILED_THIN2:
      case GX2_TILING_2B_TILED_THIN4:
      case GX2_TILING_2B_TILED_THICK:
        original_offset = ComputeSurfaceAddrFromCoordMacroTiled(x, y, 0, 0, 0, 0, 0);
        break;
      default:
        return RESULT_UNSUPPORTED_FILE_FORMAT;
      }

      qint32 new_offset;

      // Write the new pixels in their normal order to the new byte array.
      switch (m_header->format)
      {
      case GX2_FMT_BC1_UNORM:
      case GX2_FMT_BC1_SRGB:
      case GX2_FMT_BC4_UNORM:
      case GX2_FMT_BC4_SNORM:
      {
        new_offset = (y * width + x) * 8;
        break;
      }
      default:
        new_offset = (y * width + x) * 16;
        break;
      }

      // TODO: there's probably a better way of approaching this
      for (int i = 0; i < 8; i++)
      {
        if (original_offset + i > raw_image_data.size())
        {
          qDebug("Error: Tried to read pixel outside of image data. "
                 "Skipping pixel.");
          continue;
        }
        if (new_offset + i > deswizzled_image_data.size())
        {
          qDebug() << "Error: Tried to write pixel outside of image data. "
                      "Skipping pixel.";
          continue;
        }
        // qDebug("Writing raw offset %04X to %04X...", origOffset + i, newPos + i);
        deswizzled_image_data[new_offset + i] = raw_image_data[original_offset + i];
        // qDebug("Deswizzled data using at(): %0X.",
        // rawImageData.at(newPos + i));
        // qDebug("Size: %i", deswizzledImageData.size());
        //            qDebug("Done.");
      }
    }
  }
  DDS dds(&deswizzled_image_data);
  dds.MakeHeader(m_header->width, m_header->height, m_header->depth, m_header->num_mips, true,
                 m_header->format);
  dds.WriteFile(m_name);
  return RESULT_SUCCESS;
}

quint64 GX2ImageBase::ComputeSurfaceAddrFromCoordMacroTiled(quint32 x, quint32 y, quint32 slice,
                                                            quint32 sample, quint32 tileBase,
                                                            quint32 compBits, quint32* pBitPosition)
{
  // The commenting in this function will eventually be removed in favor of a dedicated document
  // explaining the whole thing.
  quint64 num_pipes = m_pipes;
  quint64 num_banks = m_banks;
  quint64 num_group_bits = m_group_bit_count;
  quint64 num_pipe_bits = m_pipe_bit_count;
  quint64 num_bank_bits = m_bank_bit_count;
  quint32 num_samples = m_num_samples;

  // Get the pixel index within the micro tile.
  quint64 pixel_index_within_micro_tile = ComputePixelIndexWithinMicroTile(x, y, slice);

  // Offset of the beginning of the current sample of the current tile.
  quint64 sample_offset_within_micro_tile;
  // Offset of the current pixel relative to the beginning of the current
  // sample.
  quint64 pixel_offset_within_sample;

  if (m_has_depth)
  {
    if (compBits && compBits != m_bpp)
    {
      sample_offset_within_micro_tile = tileBase + compBits * sample;
      pixel_offset_within_sample = num_samples * compBits * pixel_index_within_micro_tile;
    }
    else
    {
      sample_offset_within_micro_tile = m_bpp * sample;
      pixel_offset_within_sample = num_samples * m_bpp * pixel_index_within_micro_tile;
    }
  }
  else
  {
    // Number of bytes in one micro tile with all samples /
    // Number of samples in each micro tile =
    // Number of bits in one micro tile in one sample.

    // Number of bits in one micro tile in one sample *
    // Current sample we're in =
    // Offset of the start of the micro tile sample.

    // This works because the structure is like so:
    // Micro Tile 1:
    // Sample 1
    // Sample 2
    // Micro Tile 2:
    // Sample 1
    // Sample 2
    sample_offset_within_micro_tile = (m_num_micro_tile_bits / num_samples) * sample;
    // Number of bits in one pixel *
    // The pixel index =
    // The offset of the current pixel relative to the beginning of the current sample.
    pixel_offset_within_sample = m_bpp * pixel_index_within_micro_tile;
  }

  // Offset of the pixel within the sample +
  // Offset of the beginning of the current sample =
  // Pixel offset relative to the beginning of the micro tile.
  quint64 elemOffset = pixel_offset_within_sample + sample_offset_within_micro_tile;

  // ???
  if (pBitPosition != nullptr)
    *pBitPosition = static_cast<quint32>(elemOffset % 8);

  // How many samples there are in each slice
  quint64 samples_per_slice;
  // ???
  quint64 num_sample_splits;
  // Which slice the sample lies in?
  quint64 sampleSlice;
  quint64 tile_slice_bits;

  // If there's more than one sample, and one micro tile can't fit in one split.
  // TODO: some of this might be able to be moved to readimagefromdata?
  // TODO: This is currently undocumented because I have no AA textures to work off of.
  if (num_samples > 1 && m_num_micro_tile_bytes > static_cast<quint64>(m_split_size))
  {
    samples_per_slice = m_split_size / m_bytes_per_sample;
    num_sample_splits = num_samples / samples_per_slice;
    num_samples = static_cast<quint32>(samples_per_slice);

    tile_slice_bits = m_num_micro_tile_bits / num_sample_splits;
    // The recalculated pixel offset / tileSliceBits = ?
    sampleSlice = elemOffset / tile_slice_bits;
    elemOffset %= tile_slice_bits;
  }
  else
  {
    // The number of samples in each micro tile =
    // The number of samples in each slice.
    samples_per_slice = num_samples;
    // ???
    num_sample_splits = 1;
    // ???
    sampleSlice = 0;
  }

  // This might have some correlation with pBitPosition?
  elemOffset /= 8;

  // Each of the Wii U's RAM chips has 2 channels, here we select a seemingly
  // "random" one using an algorithm to generate one from the coordinates.
  quint64 pipe = ComputePipeFromCoordWoRotation(x, y);
  // The Wii U has 4 RAM chips, here we select a seemingly "random" one using an
  // algorithm to generate one from the coordinates.
  quint64 bank = ComputeBankFromCoordWoRotation(x, y);

  // Number of pipes *
  // Random bank index =
  // Random bank index, shifted over in its correct spot.
  // Shifted bank index +
  // Random pipe index =
  // Three bits containing the random bank and pipe.
  quint64 bank_pipe = pipe + num_pipes * bank;
  // Number of pipes *
  // Bank index specified by texture =
  // Bank index specified by texture, shifted over in its correct spot.
  // Shifted specified bank index +
  // Specified pipe index =
  // Three bits containing the bank and pipe specified by the texture.
  quint64 swizzle = m_pipe_swizzle + num_pipes * m_bank_swizzle;
  // The current slice the pixel is in.
  quint64 sliceIn = slice;

  if (m_is_thick_macro_tiled)
    sliceIn /= ThickTileThickness;

  // Algorithm to recalculate bank and pipe?
  bank_pipe ^= num_pipes * sampleSlice * ((num_banks >> 1) + 1) ^ (swizzle + sliceIn * rotate);
  bank_pipe %= num_pipes * num_banks;
  pipe = bank_pipe % num_pipes;
  bank = bank_pipe / num_pipes;

  m_slice_offset =
      m_slice_bytes * ((sampleSlice + num_sample_splits * slice) / m_micro_tile_thickness);

  quint64 macro_tile_index_x = x / m_macro_tile_pitch;
  quint64 macro_tile_index_y = y / m_macro_tile_height;

  // Y index of the macro tile *
  // Number of macro tiles in each row =
  // Index of the first macro tile in the current row.

  // Index of the first macro tile in the current row +
  // X index of the macro tile =
  // Index of the macro tile.

  // Number of bytes in one macro tile *
  // Index of the macro tile =
  // Offset of the macro tile.
  quint64 macro_tile_offset =
      m_num_macro_tile_bytes * (macro_tile_index_x + m_macro_tiles_per_row * macro_tile_index_y);

  // Do bank swapping if needed
  switch (m_header->tile_mode)
  {
  case GX2_TILING_2B_TILED_THIN1:
  case GX2_TILING_2B_TILED_THIN2:
  case GX2_TILING_2B_TILED_THIN4:
  case GX2_TILING_2B_TILED_THICK:
  case GX2_TILING_3B_TILED_THIN1:
  case GX2_TILING_3B_TILED_THICK:
  {
    static const quint32 bankSwapOrder[] = {0, 1, 3, 2, 6, 7, 5, 4, 0, 0};
    quint64 bank_swap_width = ComputeSurfaceBankSwappedWidth(m_header->pitch, nullptr);
    quint64 swap_index = m_macro_tile_pitch * macro_tile_index_x / bank_swap_width;
    quint64 bank_mask = m_banks - 1;
    bank ^= bankSwapOrder[swap_index & bank_mask];
    break;
  }
  default:
    break;
  }

  // Calculate final offset
  // Get mask targeting every group bit.
  quint64 group_mask = (1 << num_group_bits) - 1;
  // Offset of the macro tile +
  // Offset of the slice relative to the beginning of the macro tile =
  // Absolute offset of the slice.

  // Number of bits containing the bank +
  // Number of bits containing the pipe =
  // Number of bits containing the bank and pipe.

  // Absolute offset of the slice >>
  // Number of bits containing the bank and pipe =
  // Shifted absolute offset of the slice.

  // Pixel offset relative to the beginning of the micro tile +
  // Shifted absolute offset of the slice +
  // Offset of the pixel.
  quint64 total_offset =
      elemOffset + ((macro_tile_offset + m_slice_offset) >> (num_bank_bits + num_pipe_bits));

  // Get the part of the pixel offset left of the pipe and bank.
  quint64 offset_high = (total_offset & ~group_mask) << (num_bank_bits + num_pipe_bits);
  // Get the part of the pixel offset right of the pipe and bank.
  quint64 offset_low = total_offset & group_mask;
  // Get the actual pipe and bank.
  quint64 bankBits = bank << (num_pipe_bits + num_group_bits);
  quint64 pipeBits = pipe << num_group_bits;
  // Put it all together.
  quint64 offset = bankBits | pipeBits | offset_low | offset_high;

#ifdef DEBUG
  static int i = 0;

  if (i < 50)
  {
    //    qDebug("Bank: %llu", bank);
    //    qDebug("Pipe: %llu", pipe);
    //    qDebug("Macro Tile Offset: %04llX", macroTileOffset);

    qDebug("Bank (Bin):            %s", QString::number(bank, 2).toStdString().c_str());
    qDebug("Pipe (Bin):              %s", QString::number(pipe, 2).toStdString().c_str());
    qDebug("High Offset:     %s", QString::number(offset_high, 2).toStdString().c_str());
    qDebug("Low Offset:               %s", QString::number(offset_low, 2).toStdString().c_str());
    qDebug("Final Offset:    %s (AKA: 0x%s)", QString::number(offset, 2).toStdString().c_str(),
           QString::number(offset, 16).toStdString().c_str());
    qDebug() << "-----------------";
    i++;
  }
#endif

  return offset;
}

quint32 GX2ImageBase::ComputePixelIndexWithinMicroTile(quint32 x, quint32 y, quint32 z)
{
  quint32 pixelBit0 = 0;
  quint32 pixelBit1 = 0;
  quint32 pixelBit2 = 0;
  quint32 pixelBit3 = 0;
  quint32 pixelBit4 = 0;
  quint32 pixelBit5 = 0;
  quint32 pixelBit6 = 0;
  quint32 pixelBit7 = 0;
  quint32 pixelBit8 = 0;
  quint32 pixelNumber;

  quint32 x0 = GET_BIT(x, 0);
  quint32 x1 = GET_BIT(x, 1);
  quint32 x2 = GET_BIT(x, 2);
  quint32 y0 = GET_BIT(y, 0);
  quint32 y1 = GET_BIT(y, 1);
  quint32 y2 = GET_BIT(y, 2);
  quint32 z0 = GET_BIT(z, 0);
  quint32 z1 = GET_BIT(z, 1);
  quint32 z2 = GET_BIT(z, 2);

  if (m_micro_tile_type == GX2_MICRO_TILING_THICK_TILING)
  {
    pixelBit0 = x0;
    pixelBit1 = y0;
    pixelBit2 = z0;
    pixelBit3 = x1;
    pixelBit4 = y1;
    pixelBit5 = z1;
    pixelBit6 = x2;
    pixelBit7 = y2;
  }
  else
  {
    if (m_micro_tile_type != GX2_MICRO_TILING_DISPLAYABLE)
    {
      pixelBit0 = x0;
      pixelBit1 = y0;
      pixelBit2 = x1;
      pixelBit3 = y1;
      pixelBit4 = x2;
      pixelBit5 = y2;
    }
    else
    {
      switch (m_bpp)
      {
      case 8:
        pixelBit0 = x0;
        pixelBit1 = x1;
        pixelBit2 = x2;
        pixelBit3 = y1;
        pixelBit4 = y0;
        pixelBit5 = y2;
        break;
      case 16:
        pixelBit0 = x0;
        pixelBit1 = x1;
        pixelBit2 = x2;
        pixelBit3 = y0;
        pixelBit4 = y1;
        pixelBit5 = y2;
        break;
      case 64:
        pixelBit0 = x0;
        pixelBit1 = y0;
        pixelBit2 = x1;
        pixelBit3 = x2;
        pixelBit4 = y1;
        pixelBit5 = y2;
        break;
      case 128:
        pixelBit0 = y0;
        pixelBit1 = x0;
        pixelBit2 = x1;
        pixelBit3 = x2;
        pixelBit4 = y1;
        pixelBit5 = y2;
        break;
      case 32:
      case 96:
      default:
        pixelBit0 = x0;
        pixelBit1 = x1;
        pixelBit2 = y0;
        pixelBit3 = x2;
        pixelBit4 = y1;
        pixelBit5 = y2;
        break;
      }
    }

    if (m_micro_tile_thickness > 1)
    {
      pixelBit6 = z0;
      pixelBit7 = z1;
    }
  }

  if (m_micro_tile_thickness == 8)
  {
    pixelBit8 = z2;
  }

  pixelNumber =
      ((pixelBit0) | (pixelBit1 << 1) | (pixelBit2 << 2) | (pixelBit3 << 3) | (pixelBit4 << 4) |
       (pixelBit5 << 5) | (pixelBit6 << 6) | (pixelBit7 << 7) | (pixelBit8 << 8));

  //  qDebug() << "X: " << x << "Y: " << y << "Pixel Index: " << pixelNumber;

  return pixelNumber;
}

void GX2ImageBase::ComputeSurfaceThickness()
{
  switch (m_header->tile_mode)
  {
  case GX2_TILING_1D_TILED_THICK:
  case GX2_TILING_2D_TILED_THICK:
  case GX2_TILING_2B_TILED_THICK:
  case GX2_TILING_3D_TILED_THICK:
  case GX2_TILING_3B_TILED_THICK:
    m_micro_tile_thickness = 4;
  case GX2_TILING_2D_TILED_XTHICK:
  case GX2_TILING_3D_TILED_XTHICK:
    m_micro_tile_thickness = 8;
  default:
    m_micro_tile_thickness = 1;
  }
}

void GX2ImageBase::ComputeSurfaceRotationFromTileMode()
{
  switch (m_header->tile_mode)
  {
  case GX2_TILING_2D_TILED_THIN1:
  case GX2_TILING_2D_TILED_THIN2:
  case GX2_TILING_2D_TILED_THIN4:
  case GX2_TILING_2D_TILED_THICK:
  case GX2_TILING_2B_TILED_THIN1:
  case GX2_TILING_2B_TILED_THIN2:
  case GX2_TILING_2B_TILED_THIN4:
  case GX2_TILING_2B_TILED_THICK:
    rotate = m_pipes * ((m_banks >> 1) - 1);
    break;
  case GX2_TILING_3D_TILED_THIN1:
  case GX2_TILING_3D_TILED_THICK:
  case GX2_TILING_3B_TILED_THIN1:
  case GX2_TILING_3B_TILED_THICK:
    if (m_pipes >= 4)
    {
      rotate = (m_pipes >> 1) - 1;
    }
    else
    {
      rotate = 1;
    }
    break;
  default:
    rotate = 0;
  }
}

void GX2ImageBase::ComputeThickMicroTiling()
{
  switch (m_header->tile_mode)
  {
  case GX2_TILING_2D_TILED_THICK:
  case GX2_TILING_2B_TILED_THICK:
  case GX2_TILING_3D_TILED_THICK:
  case GX2_TILING_3B_TILED_THICK:
    m_is_thick_macro_tiled = true;
  default:
    m_is_thick_macro_tiled = false;
  }
}

void GX2ImageBase::ComputeBankSwappedTileMode()
{
  switch (m_header->tile_mode)
  {
  case GX2_TILING_2B_TILED_THIN1:
  case GX2_TILING_2B_TILED_THIN2:
  case GX2_TILING_2B_TILED_THIN4:
  case GX2_TILING_2B_TILED_THICK:
  case GX2_TILING_3B_TILED_THIN1:
  case GX2_TILING_3B_TILED_THICK:
    bankSwappedTileMode = true;
  default:
    bankSwappedTileMode = false;
  }
}

quint32 GX2ImageBase::ComputeMacroTileAspectRatio()
{
  switch (m_header->tile_mode)
  {
  case GX2_TILING_2B_TILED_THIN1:
  case GX2_TILING_3D_TILED_THIN1:
  case GX2_TILING_3B_TILED_THIN1:
    return 1;
  case GX2_TILING_2D_TILED_THIN2:
  case GX2_TILING_2B_TILED_THIN2:
    return 2;
  case GX2_TILING_2D_TILED_THIN4:
  case GX2_TILING_2B_TILED_THIN4:
    return 4;
  default:
    return 1;
  }
}

quint32 GX2ImageBase::ComputeSurfaceBankSwappedWidth(quint32 pitch, quint32* pSlicesPerTile)
{
  auto bankSwapWidth = quint32{0};
  auto numBanks = m_banks;
  auto numPipes = m_pipes;
  // TODO: find the right value for these:
  auto swapSize = m_swap_size;
  auto rowSize = m_row_size;
  auto splitSize = m_split_size;
  auto groupSize = m_pipe_interleave_bytes;
  auto slicesPerTile = quint32{1};
  auto bytesPerSample = 8 * m_bpp & 0x1FFFFFFF;
  auto samplesPerTile = splitSize / bytesPerSample;

  if (splitSize / bytesPerSample)
  {
    slicesPerTile = std::max<quint32>(1u, m_num_samples / samplesPerTile);
  }

  if (pSlicesPerTile)
  {
    *pSlicesPerTile = slicesPerTile;
  }

  quint32 num_samples = m_num_samples;

  if (m_is_thick_macro_tiled)
  {
    num_samples = 4;
  }

  auto bytesPerTileSlice = num_samples * bytesPerSample / slicesPerTile;

  if (bankSwappedTileMode)
  {
    auto factor = ComputeMacroTileAspectRatio();
    auto swapTiles = std::max<quint32>(1u, (swapSize >> 1) / m_bpp);
    auto swapWidth = swapTiles * 8 * numBanks;
    auto heightBytes = num_samples * factor * numPipes * m_bpp / slicesPerTile;
    auto swapMax = numPipes * numBanks * rowSize / heightBytes;
    auto swapMin = groupSize * 8 * numBanks / bytesPerTileSlice;

    bankSwapWidth = std::min(swapMax, std::max(swapMin, swapWidth));

    while (bankSwapWidth >= 2 * pitch)
    {
      bankSwapWidth >>= 1;
    }
  }

  return bankSwapWidth;
}

quint32 GX2ImageBase::ComputePipeFromCoordWoRotation(quint32 x, quint32 y)
{
  quint32 pipe;
  quint32 pipeBit0 = 0;
  quint32 pipeBit1 = 0;
  quint32 pipeBit2 = 0;

  quint32 x3 = GET_BIT(x, 3);
  quint32 x4 = GET_BIT(x, 4);
  quint32 x5 = GET_BIT(x, 5);
  quint32 y3 = GET_BIT(y, 3);
  quint32 y4 = GET_BIT(y, 4);
  quint32 y5 = GET_BIT(y, 5);

  switch (m_banks)
  {
  case 1:
    pipeBit0 = 0;
    break;
  case 2:
  case 4:
    pipeBit0 = (y3 ^ x3);
    break;
  case 8:
    pipeBit0 = (y3 ^ x5);
    pipeBit1 = (y4 ^ x5 ^ x4);
    pipeBit2 = (y5 ^ x3);
    break;
  default:
    pipe = 0;
    break;
  }

  pipe = pipeBit0 | (pipeBit1 << 1) | (pipeBit2 << 2);
  return pipe;
}

quint32 GX2ImageBase::ComputeBankFromCoordWoRotation(quint32 x, quint32 y)
{
  quint32 numPipes = m_pipes;
  quint32 numBanks = m_banks;
  // TODO: get the right value for this
  quint32 bankOpt = 0;

  quint32 tx = x / numBanks;
  quint32 ty = y / numPipes;

  quint32 bank;
  quint32 bankBit0 = 0;
  quint32 bankBit1 = 0;
  quint32 bankBit2 = 0;

  quint32 x3 = GET_BIT(x, 3);
  quint32 x4 = GET_BIT(x, 4);
  quint32 x5 = GET_BIT(x, 5);

  quint32 tx3 = GET_BIT(tx, 3);

  quint32 ty3 = GET_BIT(ty, 3);
  quint32 ty4 = GET_BIT(ty, 4);
  quint32 ty5 = GET_BIT(ty, 5);

  switch (m_banks)
  {
  case 4:
    bankBit0 = ((y / (16 * numPipes)) ^ x3) & 1;

    if (bankOpt == 1 && numPipes == 8)
    {
      bankBit0 ^= x / 0x20 & 1;
    }

    bank = bankBit0 | 2 * (((y / (8 * numPipes)) ^ x4) & 1);
    break;
  case 8:
    bankBit0 = (ty5 ^ x3);

    if (bankOpt == 1 && numPipes == 8)
    {
      bankBit0 ^= tx3;
    }

    bankBit1 = (ty5 ^ ty4 ^ x4);
    bankBit2 = (ty3 ^ x5);
    bank = bankBit0 | (bankBit1 << 1) | (bankBit2 << 2);
    break;
  }
  return bank;
}

void GX2ImageBase::SetName(const QString& value)
{
  m_name = value;
}
