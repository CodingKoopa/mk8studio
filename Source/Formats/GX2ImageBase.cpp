#include "GX2ImageBase.h"

#include "DDS.h"

ResultCode GX2ImageBase::ReadImageFromData()
{
  // Default to GX2_SURFACE_FORMAT_INVALID.
  m_format_info = m_format_info_list[0];

  for (int i = 0; i < m_format_info_list.size(); i++)
  {
    if (m_header->format == static_cast<quint32>(m_format_info_list[i].id))
    {
      m_format_info = m_format_info_list[i];
      break;
    }
  }

  m_format_info.shared_info = m_shared_format_info_list[0];

  for (int i = 0; i < m_shared_format_info_list.size(); i++)
  {
    if (m_format_info.format == m_shared_format_info_list[i].format)
    {
      m_format_info.shared_info = m_shared_format_info_list[i];
      break;
    }
  }

  m_tile_mode_info = m_tile_mode_info_list[m_header->tile_mode];

  for (int i = 0; i < m_shared_tile_mode_info_list.size(); i++)
  {
    if (m_tile_mode_info.mode == m_shared_tile_mode_info_list[i].mode)
    {
      m_tile_mode_info.shared_info = m_shared_tile_mode_info_list[i];
      break;
    }
  }

  // Temporary hack to find special textures.
  if (m_header->aa_mode != 0 || static_cast<quint32>(m_tile_mode_info.thickness) > 1 ||
      m_format_info.name == "GX2_SURFACE_FORMAT_INVALID" ||
      m_format_info.shared_info.format == Format::Invalid)
    return RESULT_UNSUPPORTED_FILE_FORMAT_IMPORTANT;

  quint32 width = m_header->width;
  quint32 height = m_header->height;

  m_num_samples = 1 << m_header->aa_mode;

  m_pipe_swizzle = GetBit(m_header->swizzle, 8);
  m_bank_swizzle = GetBits(m_header->swizzle, 9, 3);

  if (m_format_info.shared_info.use == SharedFormatInfo::Use::DepthBuffer)
    m_has_depth = true;
  else
    m_has_depth = false;

  switch (m_format_info.format)
  {
  case Format::BC1:
  case Format::BC4:
  {
    // TODO: Add bpp to FormatInfo
    //    m_bpp = 0x40 / 16;
    width /= 4;
    height /= 4;
    break;
  }
  case Format::BC5:
  {
    //    m_bpp = 0x80;
    width /= 4;
    height /= 4;
    break;
  }
  default:
    return RESULT_UNSUPPORTED_FILE_FORMAT;
  }

  switch (m_tile_mode_info.mode)
  {
  // Macro Tiled
  // TODO: Add proper support for 3D textures.
  case TileMode::Macro:
    // Number of banks *
    // 8 (?) =
    // Physical width of a row in one macro tile.
    m_macro_tile_pitch = m_num_banks * 8;
    // Number of pipes *
    // 8 (?) =
    // Height of one macro tile.
    m_macro_tile_height = m_num_pipes * 8;

    if (m_tile_mode_info.aspect_ratio == 2)
    {
      // Physical width of a row in one macro tile /
      // 2 =
      // Physical width of a row in one thin 2 (?) macro tile.
      m_macro_tile_pitch /= 2;
      // Height of one macro tile. *
      // 2 =
      // Height of one thin 2 (?) macro tile.
      m_macro_tile_height *= 2;
    }
    else if (m_tile_mode_info.aspect_ratio == 4)
    {
      // Physical width of a row in one macro tile /
      // 4 =
      // Physical width of a row in one thin 4 (?) macro tile.
      m_macro_tile_pitch /= 4;
      // Height of one macro tile. *
      // 4 =
      // Height of one thin 4 (?) macro tile.
      m_macro_tile_height *= 4;
    }

    // Physical width of a row of the whole texture /
    // Physical width of a row in one macro tile =
    // Number of macro tiles in a row of the whole texture.
    m_macro_tiles_per_row = m_header->pitch / m_macro_tile_pitch;

    // Physical width of a row in one macro tile *
    // Height of one macro tile =
    // Number of pixels in one macro tile.

    // Number of pixels in one macro tile *
    // Number of bits in one pixel =
    // Number of bits in one sample of a macro tile.

    // Number of bits in one sample of a macro tile. *
    // Thickness of one micro tile =
    // Number of bits in one sample of a macro tile including thickness.

    // Number of bits in one sample of a macro tile including thickness *
    // Number of samples =
    // Number of bits in all samples of a macro tile including thickness.

    // Number of bits in all samples of a macro tile including thickness -> Bytes =
    // Number of bytes in one macro tile (All .
    m_num_macro_tile_bytes =
        BitsToBytes(m_macro_tile_pitch * m_macro_tile_height * m_format_info.shared_info.bpp *
                    static_cast<quint32>(m_tile_mode_info.thickness) * m_num_samples);

    // Physical width of a row of the whole texture *
    // Height of the whole texture =
    // Number of pixels in the whole texture.

    // Number of pixels in the whole texture *
    // Number of bits in one pixel =
    // Number of bits in the whole texture without sampling and thickness.

    // Number of bits in the whole texture without sampling and thickness *
    // Thickness of one micro tile =
    // Number of bits in the whole texture without sampling.

    // Number of bits in the whole texture without sampling *
    // Number of samples =
    // Number of bits in the whole texture.

    // Number of bits in the whole texture -> Bytes =
    // Number of bytes in the whole texture.
    m_num_slice_bytes =
        BitsToBytes(m_header->pitch * m_header->height * m_format_info.shared_info.bpp *
                    static_cast<quint32>(m_tile_mode_info.thickness) * m_num_samples);

  // Micro Tiled
  case TileMode::Micro:
    // TODO: figure this out
    if (true)
      m_micro_tile_type = MicroTileType::GX2_MICRO_TILING_DISPLAYABLE;
    else
      m_micro_tile_type = MicroTileType::GX2_MICRO_TILING_DISPLAYABLE;

    // Number of pixels in one micro tile *
    // Number of bits in one pixel =
    // Number of bits in one sample of a micro tile.

    // Number of bits in one sample of a micro tile. *
    // Thickness of one micro tile =
    // Number of bits in one sample of a micro tile including thickness.

    // Number of bits in one sample of a micro tile including thickness *
    // Number of samples =
    // Number of bits in all samples of a micro tile including thickness.

    // Number of bits in all samples of a micro tile including thickness -> Bytes =
    // Number of bytes in one micro tile.
    m_num_micro_tile_bits = m_num_micro_tile_pixels * m_format_info.shared_info.bpp *
                            static_cast<quint32>(m_tile_mode_info.thickness) * m_num_samples;

    // Number of bits in one micro tile /
    // Number of bits in a byte =
    // Number of bytes in one micro tile with all samples.
    // TODO: lambda for conversion to bits or something?
    m_num_micro_tile_bytes = m_num_micro_tile_bits / 8;

    // Number of bytes in one micro tile /
    // Number of samples in each micro tile =
    // Number of bytes in one sample of a micro tile.
    m_bytes_per_sample = m_num_micro_tile_bytes / m_num_samples;
    break;

  case TileMode::Linear:
  default:
    return RESULT_UNSUPPORTED_FILE_FORMAT;
  }

  if (m_has_depth)
    m_micro_tile_type = MicroTileType::GX2_MICRO_TILING_NON_DISPLAYABLE;
  else
    m_micro_tile_type = MicroTileType::GX2_MICRO_TILING_DISPLAYABLE;

  m_deswizzled_image_data = new QByteArray();
  m_deswizzled_image_data->resize(m_header->data_length);
  m_deswizzled_image_data->fill(0);

  for (quint32 y = 0; y < height; ++y)
  {
    for (quint32 x = 0; x < width; ++x)
    {
      qint32 original_offset;
      // Get the offset of the pixel at the current coordinate.
      switch (m_tile_mode_info.mode)
      {
      case TileMode::Macro:
        original_offset = ComputeSurfaceAddrFromCoordMacroTiled(x, y, 0, 0, 0, 0);
        break;
      default:
        return RESULT_UNSUPPORTED_FILE_FORMAT;
      }
      quint32 block_size;

      // Write the new pixels in their normal order to the new byte array.
      switch (m_format_info.format)
      {
      case Format::BC1:
      case Format::BC4:
      {
        block_size = 8;
        break;
      }
      default:
        block_size = 16;
        break;
      }
      qint32 new_offset = (y * width + x) * block_size;

      if (original_offset > m_raw_image_data->size())
      {
        qDebug("Error: Tried to read pixel outside of image data. "
               "Skipping pixel.");
        continue;
      }
      if (new_offset > m_deswizzled_image_data->size())
      {
        qDebug() << "Error: Tried to write pixel outside of image data. "
                    "Skipping pixel.";
        continue;
      }
      m_deswizzled_image_data->replace(new_offset, block_size,
                                       m_raw_image_data->constData() + original_offset, block_size);
      // qDebug("Writing raw offset %04X to %04X...", origOffset + i, newPos + i);

      // qDebug("Deswizzled data using at(): %0X.",
      // rawImageData.at(newPos + i));
      // qDebug("Size: %i", deswizzledImageData.size());
      //            qDebug("Done.");
    }
  }
  return RESULT_SUCCESS;
}

ResultCode GX2ImageBase::ExportToDDS()
{
  DDS dds(m_deswizzled_image_data);
  dds.MakeHeader(m_header->width, m_header->height, m_header->depth, m_header->num_mips, true,
                 m_format_info);
  dds.WriteFile(m_name);
  return RESULT_SUCCESS;
}

quint64 GX2ImageBase::ComputeSurfaceAddrFromCoordMacroTiled(quint32 x, quint32 y, quint32 slice,
                                                            quint32 sample, quint32 tile_base,
                                                            quint32 comp_bits)
{
  // The commenting in this function will eventually be removed in favor of a dedicated document
  // explaining the whole thing.

  // TODO: Using m_num_samples directly might be possible. Needs testing with AA surfaces.
  quint32 num_samples = m_num_samples;

  // Get the pixel index within the micro tile.
  quint64 pixel_index_within_micro_tile = ComputePixelIndexWithinMicroTile(x, y, slice);

  // Offset of the beginning of the current sample of the current tile.
  quint64 sample_offset_within_micro_tile;
  // Offset of the current pixel relative to the beginning of the current sample.
  quint64 pixel_offset_within_sample;

  if (m_has_depth)
  {
    if (comp_bits && comp_bits != m_format_info.shared_info.bpp)
    {
      sample_offset_within_micro_tile = tile_base + comp_bits * sample;
      pixel_offset_within_sample = num_samples * comp_bits * pixel_index_within_micro_tile;
    }
    else
    {
      // Number of bits in one pixel *
      // Current sample we're in =
      //
      sample_offset_within_micro_tile = m_format_info.shared_info.bpp * sample;
      pixel_offset_within_sample =
          num_samples * m_format_info.shared_info.bpp * pixel_index_within_micro_tile;
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

    // This works because I think the structure is like so:
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
    pixel_offset_within_sample = m_format_info.shared_info.bpp * pixel_index_within_micro_tile;
  }

  // Offset of the pixel within the sample +
  // Offset of the beginning of the current sample =
  // Pixel offset relative to the beginning of the micro tile.
  quint64 elem_offset = pixel_offset_within_sample + sample_offset_within_micro_tile;

  // How many samples there are in each slice
  quint64 samples_per_slice;
  // ???
  quint64 num_sample_splits;
  // Which slice the sample lies in?
  quint64 sampleSlice;
  quint64 tile_slice_bits;

  // If there's more than one sample, and one micro tile can't fit in one split.
  // TODO: some of this might be able to be moved to readimagefromdata?
  // TODO: This is currently undocumented because I have no AA or thick textures to work off of.
  if (num_samples > 1 && m_num_micro_tile_bytes > static_cast<quint64>(m_split_size))
  {
    samples_per_slice = m_split_size / m_bytes_per_sample;
    num_sample_splits = num_samples / samples_per_slice;
    // TODO: Could this be written directly to m_num_samples?
    num_samples = static_cast<quint32>(samples_per_slice);

    tile_slice_bits = m_num_micro_tile_bits / num_sample_splits;
    sampleSlice = elem_offset / tile_slice_bits;
    elem_offset %= tile_slice_bits;
  }
  else
  {
    samples_per_slice = num_samples;
    num_sample_splits = 1;
    sampleSlice = 0;
  }

  // This might have some correlation with pBitPosition?
  elem_offset /= 8;

  quint32 x_3 = GetBit(x, 3);
  quint32 x_4 = GetBit(x, 4);
  quint32 x_5 = GetBit(x, 5);

  quint32 y_3 = GetBit(y, 3);
  quint32 y_4 = GetBit(y, 4);
  quint32 y_5 = GetBit(y, 5);

  quint32 tile_x = x / m_num_banks;
  quint32 tile_y = y / m_num_pipes;

  quint32 tile_x_3 = GetBit(tile_x, 3);

  quint32 tile_y_3 = GetBit(tile_y, 3);
  quint32 tile_y_4 = GetBit(tile_y, 4);
  quint32 tile_y_5 = GetBit(tile_y, 5);

  // Each of the Wii U's RAM chips has 2 channels, here we select a seemingly
  // "random" one using an algorithm to generate one from the coordinates.
  quint32 pipe;
  quint32 pipe_bit_0 = 0;
  quint32 pipe_bit_1 = 0;
  quint32 pipe_bit_2 = 0;

  switch (m_num_banks)
  {
  case 1:
    pipe_bit_0 = 0;
    break;
  case 2:
  case 4:
    pipe_bit_0 = (y_3 ^ x_3);
    break;
  case 8:
    pipe_bit_0 = (y_3 ^ x_5);
    pipe_bit_1 = (y_4 ^ x_5 ^ x_4);
    pipe_bit_2 = (y_5 ^ x_3);
    break;
  default:
    pipe = 0;
    break;
  }

  pipe = pipe_bit_0 | (pipe_bit_1 << 1) | (pipe_bit_2 << 2);

  // The Wii U has 4 RAM chips, here we select a seemingly "random" one using an
  // algorithm to generate one from the coordinates.
  // TODO: get the right value for this
  quint32 bankOpt = 0;

  quint32 bank;
  quint32 bankBit0 = 0;
  quint32 bankBit1 = 0;
  quint32 bankBit2 = 0;

  switch (m_num_banks)
  {
  case 4:
    bankBit0 = ((y / (16 * m_num_pipes)) ^ x_3) & 1;

    if (bankOpt == 1 && m_num_pipes == 8)
    {
      bankBit0 ^= x / 0x20 & 1;
    }

    bank = bankBit0 | 2 * (((y / (8 * m_num_pipes)) ^ x_4) & 1);
    break;
  case 8:
    bankBit0 = (tile_y_5 ^ x_3);

    if (bankOpt == 1 && m_num_pipes == 8)
    {
      bankBit0 ^= tile_x_3;
    }

    bankBit1 = (tile_y_5 ^ tile_y_4 ^ x_4);
    bankBit2 = (tile_y_3 ^ x_5);
    bank = bankBit0 | (bankBit1 << 1) | (bankBit2 << 2);
    break;
  }

  // Number of pipes *
  // Random bank index =
  // Random bank index, shifted over in its correct spot.

  // Shifted bank index +
  // Random pipe index =
  // Three bits containing the random bank and pipe.
  quint64 bank_pipe = bank * m_num_pipes + pipe;

  // Number of pipes *
  // Bank index specified by texture =
  // Bank index specified by texture, shifted over in its correct spot.

  // Shifted specified bank index +
  // Specified pipe index =
  // Three bits containing the bank and pipe specified by the texture.
  quint64 swizzle = m_num_pipes * m_bank_swizzle + m_pipe_swizzle;

  // The current slice the pixel is in, for 3D textures.
  quint64 sliceIn = slice;

  if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
    sliceIn /= static_cast<quint32>(m_tile_mode_info.thickness);

  // Algorithm to recalculate bank and pipe?
  bank_pipe ^= m_num_pipes * sampleSlice * ((m_num_banks >> 1) + 1) ^
               (swizzle + sliceIn * m_tile_mode_info.shared_info.rotation);
  bank_pipe %= m_num_pipes * m_num_banks;
  pipe = bank_pipe % m_num_pipes;
  bank = bank_pipe / m_num_pipes;

  m_slice_offset = m_num_slice_bytes * ((sampleSlice + num_sample_splits * slice) /
                                        static_cast<quint32>(m_tile_mode_info.thickness));

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
  if (m_tile_mode_info.swap_banks)
  {
    static const quint32 bankSwapOrder[] = {0, 1, 3, 2, 6, 7, 5, 4, 0, 0};
    quint64 bank_swap_width = ComputeSurfaceBankSwappedWidth(m_header->pitch);
    quint64 swap_index = m_macro_tile_pitch * macro_tile_index_x / bank_swap_width;
    quint64 bank_mask = m_num_banks - 1;
    bank ^= bankSwapOrder[swap_index & bank_mask];
  }
  // Calculate final offset
  // Get mask targeting every group bit.
  quint64 group_mask = (1 << m_group_bit_count) - 1;
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
      elem_offset + ((macro_tile_offset + m_slice_offset) >> (m_bank_bit_count + m_pipe_bit_count));

  // Get the part of the pixel offset left of the pipe and bank.
  quint64 offset_high = (total_offset & ~group_mask) << (m_bank_bit_count + m_pipe_bit_count);
  // Get the part of the pixel offset right of the pipe and bank.
  quint64 offset_low = total_offset & group_mask;
  // Get the actual pipe and bank.
  quint64 bank_bits = bank << (m_pipe_bit_count + m_group_bit_count);
  quint64 pipe_bits = pipe << m_group_bit_count;
  // Put it all together.
  quint64 offset = bank_bits | pipe_bits | offset_low | offset_high;

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

  quint32 x0 = GetBit(x, 0);
  quint32 x1 = GetBit(x, 1);
  quint32 x2 = GetBit(x, 2);
  quint32 y0 = GetBit(y, 0);
  quint32 y1 = GetBit(y, 1);
  quint32 y2 = GetBit(y, 2);
  quint32 z0 = GetBit(z, 0);
  quint32 z1 = GetBit(z, 1);
  quint32 z2 = GetBit(z, 2);

  if (m_micro_tile_type == MicroTileType::GX2_MICRO_TILING_THICK_TILING)
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
    if (m_micro_tile_type != MicroTileType::GX2_MICRO_TILING_DISPLAYABLE)
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
      switch (m_format_info.shared_info.bpp)
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

    if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
    {
      pixelBit6 = z0;
      pixelBit7 = z1;
    }
  }

  if (static_cast<quint32>(m_tile_mode_info.thickness) == 8)
  {
    pixelBit8 = z2;
  }

  pixelNumber =
      ((pixelBit0) | (pixelBit1 << 1) | (pixelBit2 << 2) | (pixelBit3 << 3) | (pixelBit4 << 4) |
       (pixelBit5 << 5) | (pixelBit6 << 6) | (pixelBit7 << 7) | (pixelBit8 << 8));

  return pixelNumber;
}

quint32 GX2ImageBase::ComputeSurfaceBankSwappedWidth(quint32 pitch)
{
  quint32 bank_swap_width = 0;
  quint32 slices_per_tile = 1;

  quint32 num_samples = m_num_samples;
  // TODO: Same as m_bytes_per_sample?
  quint32 bytes_per_sample = 8 * m_format_info.shared_info.bpp;
  quint32 samples_per_tile = m_split_size / bytes_per_sample;

  if (m_split_size / m_bytes_per_sample)
  {
    slices_per_tile = qMax<uint32_t>(1u, num_samples / samples_per_tile);
  }

  if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
    num_samples = 4;

  // Number of samples *
  //
  quint32 bytes_per_tile_slice = num_samples * m_bytes_per_sample / slices_per_tile;

  if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
  {
    auto swapTiles = qMax<uint32_t>(1u, (m_swap_size >> 1) / m_format_info.shared_info.bpp);
    quint64 swapWidth = swapTiles * 8 * m_num_banks;
    auto heightBytes = num_samples * m_tile_mode_info.aspect_ratio * m_num_pipes *
                       m_format_info.shared_info.bpp / slices_per_tile;
    quint64 swapMax = m_num_pipes * m_num_banks * m_row_size / heightBytes;
    quint64 swapMin = m_pipe_interleave_bytes * 8 * m_num_banks / bytes_per_tile_slice;

    bank_swap_width = qMin(swapMax, qMax(swapMin, swapWidth));

    while (bank_swap_width >= 2 * pitch)
    {
      bank_swap_width >>= 1;
    }
  }

  return bank_swap_width;
}
