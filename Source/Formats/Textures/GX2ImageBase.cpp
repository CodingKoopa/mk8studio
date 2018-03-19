#include "Formats/Textures/GX2ImageBase.h"

#include "Formats/Textures/DDS.h"

GX2ImageBase::GX2ImageBase(std::shared_ptr<File> file, quint32 start_offset, quint32 header_size)
    : FormatBase(file, start_offset, header_size)
{
}

ResultCode GX2ImageBase::SetupInfo()
{
  try
  {
    m_format_info = m_format_infos.at(m_base_header.format);
    m_common_format_info = m_common_format_infos.at(m_format_info.common_format);

    m_tile_mode_info = m_tile_mode_infos.at(m_base_header.tile_mode);
    m_common_tile_mode_rotation = m_common_tile_mode_rotations.at(m_tile_mode_info.mode);
  }
  catch (std::out_of_range)
  {
    return ResultCode::UnsupportedTextureFormat;
  }
  return ResultCode::Success;
}

ResultCode GX2ImageBase::ReadImageFromData()
{
  return CopyImage(&m_raw_image_data, &m_deswizzled_image_data, false);
}

ResultCode GX2ImageBase::WriteDeswizzledImageToData()
{
  return CopyImage(&m_deswizzled_image_data, &m_raw_image_data, true);
}

ResultCode GX2ImageBase::ImportDDS(QString path)
{
  DDS dds;
  dds.SetPath(path);
  dds.ReadFile();
  m_deswizzled_image_data = dds.GetImageData();
  return WriteDeswizzledImageToData();
}

ResultCode GX2ImageBase::ExportToDDS(QString path)
{
  DDS dds;
  dds.SetPath(path);
  dds.SetImageData(m_deswizzled_image_data);
  int bytes_written =
      dds.WriteFile(m_base_header.width, m_base_header.height, m_base_header.depth,
                    m_base_header.num_mips, m_element_size, m_format_info, m_common_format_info);
  if (bytes_written == 0)
    return ResultCode::NoBytesWritten;
  else
    return ResultCode::Success;
}

std::map<quint32, GX2ImageBase::FormatInfo> GX2ImageBase::GetFormatInfos()
{
  return m_format_infos;
}

std::map<quint32, GX2ImageBase::TileModeInfo> GX2ImageBase::GetTileModeInfos()
{
  return m_tile_mode_infos;
}

const GX2ImageBase::FormatInfo& GX2ImageBase::GetFormatInfo() const
{
  return m_format_info;
}

const GX2ImageBase::TileModeInfo& GX2ImageBase::GetTileModeInfo() const
{
  return m_tile_mode_info;
}

ResultCode GX2ImageBase::SetupInfo()
{
  try
  {
    m_format_info = m_format_infos.at(m_base_header.format);
    m_common_format_info = m_common_format_infos.at(m_format_info.common_format);

    m_tile_mode_info = m_tile_mode_infos.at(m_base_header.tile_mode);
    m_common_tile_mode_rotation = m_common_tile_mode_rotations.at(m_tile_mode_info.mode);
  }
  catch (std::out_of_range)
  {
    return ResultCode::UnsupportedTextureFormat;
  }
  return ResultCode::Success;
}

ResultCode GX2ImageBase::CopyImage(QByteArray* source, QByteArray* destination, bool swizzle)
{
  // Temporary hack to find special textures.
  if (m_base_header.aa_mode != 0 || static_cast<quint32>(m_tile_mode_info.thickness) > 1 ||
      m_format_info.name == "GX2_SURFACE_FORMAT_INVALID" ||
      m_format_info.common_format == CommonFormat::Invalid)
    return ResultCode::ImportantUnsupportedFileFormat;

  // Set up dimensions.
  quint32 width = m_base_header.width;
  quint32 height = m_base_header.height;
  if (m_common_format_info.compressed)
  {
    // Split into 4 by 4 compressed blocks of pixels.
    width /= 4;
    height /= 4;
  }

  // The size of the element, the size of 1 tile in a compressed texture, or the size of 1 pixel in
  // a decompressed texture.
  m_element_size = m_common_format_info.bpp / 8;

  m_num_samples = 1 << m_base_header.aa_mode;

  switch (m_tile_mode_info.mode)
  {
  // Macro Tiled
  // TODO: Add support for 3D textures.
  case CommonTileMode::Macro:
    // Calculate the size of the macro tiles, determined by the number of pipes/banks, and the
    // aspect ratio.

    m_macro_tile_pitch = (NUM_BANKS * 8) / m_tile_mode_info.aspect_ratio;
    m_macro_tile_height = (NUM_PIPES * 8) * m_tile_mode_info.aspect_ratio;

    // Physical width of a row of the whole texture /
    // Physical width of a row in one macro tile =
    // Number of macro tiles in a row of the whole texture.
    m_macro_tiles_per_row = m_base_header.pitch / m_macro_tile_pitch;

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
    // Number of bytes in one macro tile (All samples).
    m_num_macro_tile_bytes =
        BitsToBytes(m_macro_tile_pitch * m_macro_tile_height * m_common_format_info.bpp *
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
        BitsToBytes(m_base_header.pitch * m_base_header.height * m_common_format_info.bpp *
                    static_cast<quint32>(m_tile_mode_info.thickness) * m_num_samples);

    m_pipe_swizzle = Bit(m_base_header.swizzle, 8);

    m_bank_swizzle = GetBits(m_base_header.swizzle, 9, 3);

  // Fallthrough
  // (This comment is here to prevent compiler warnings.)

  // Micro Tiled
  case CommonTileMode::Micro:
    if (m_common_format_info.use == CommonFormatInfo::Use::DepthBuffer)
      m_micro_tile_type = MicroTileType::NonDisplayable;
    else
      m_micro_tile_type = MicroTileType::Displayable;

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
    m_num_micro_tile_bits = NUM_MICRO_TILE_PIXELS * m_common_format_info.bpp *
                            static_cast<quint32>(m_tile_mode_info.thickness) * m_num_samples;

    // Number of bits in one micro tile /
    // Number of bits in a byte =
    // Number of bytes in one micro tile with all samples.
    m_num_micro_tile_bytes = BytesToBits(m_num_micro_tile_bits);

    // Number of bytes in one micro tile /
    // Number of samples in each micro tile =
    // Number of bytes in one sample of a micro tile.
    m_bytes_per_sample = m_num_micro_tile_bytes / m_num_samples;
    break;

  case CommonTileMode::Linear:
  default:
    return ResultCode::UnsupportedTextureFormat;
  }

  destination->resize(m_base_header.data_length);
  destination->fill(0);

  for (quint32 y = 0; y < height; ++y)
  {
    for (quint32 x = 0; x < width; ++x)
    {
      qint32 original_offset = 0;
      // Get the offset of the pixel at the current coordinate.
      switch (m_tile_mode_info.mode)
      {
      case CommonTileMode::Macro:
        original_offset = GetPixelOffsetMacroTiled(x, y, 0, 0, 0, 0);
        break;
      default:
        return ResultCode::UnsupportedTextureFormat;
      }

      qint32 new_offset = (y * width + x) * m_element_size;

      if (original_offset > source->size())
      {
        qDebug("Error: Tried to read block outside of image data. "
               "Skipping pixel.");
        continue;
      }
      if (new_offset > destination->size())
      {
        qDebug() << "Error: Tried to write block outside of image data. "
                    "Skipping pixel.";
        continue;
      }
      if (swizzle)
        destination->replace(original_offset, m_element_size, source->constData() + new_offset,
                             m_element_size);
      else
        destination->replace(new_offset, m_element_size, source->constData() + original_offset,
                             m_element_size);
    }
  }
  return ResultCode::Success;
}

quint64 GX2ImageBase::GetPixelOffsetMacroTiled(quint32 x, quint32 y, quint32 slice, quint32 sample,
                                               quint32 tile_base, quint32 comp_bits)
{
  // The commenting in this function will eventually be removed in favor of a dedicated document
  // explaining the whole thing.

  /// @todo Using m_num_samples directly might be possible. Needs testing with AA surfaces.
  quint32 num_samples = m_num_samples;

  // Get the pixel index within the micro tile.
  quint64 pixel_index_within_micro_tile = GetPixelIndexMicroTiled(x, y, slice);

  // Offset of the beginning of the current sample of the current tile.
  quint64 sample_offset_within_micro_tile;
  // Offset of the current pixel relative to the beginning of the current sample.
  quint64 pixel_offset_within_sample;

  if (m_common_format_info.use == CommonFormatInfo::Use::DepthBuffer)
  {
    if (comp_bits && comp_bits != m_common_format_info.bpp)
    {
      sample_offset_within_micro_tile = tile_base + comp_bits * sample;
      pixel_offset_within_sample = num_samples * comp_bits * pixel_index_within_micro_tile;
    }
    else
    {
      sample_offset_within_micro_tile = m_common_format_info.bpp * sample;
      pixel_offset_within_sample =
          num_samples * m_common_format_info.bpp * pixel_index_within_micro_tile;
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
    pixel_offset_within_sample = m_common_format_info.bpp * pixel_index_within_micro_tile;
  }

  // Offset of the pixel within the sample +
  // Offset of the beginning of the current sample =
  // Element Offset. (Pixel offset relative to the beginning of the micro tile.)
  quint64 elem_offset = pixel_offset_within_sample + sample_offset_within_micro_tile;

  // How many samples there are in each slice
  quint64 samples_per_slice;
  // ???
  quint64 num_sample_splits;
  // Which slice the sample lies in?
  quint64 sample_slice;
  quint64 tile_slice_bits;

  // If there's more than one sample, and one micro tile can't fit in one split.
  /// @todo Some of this might be able to be moved to readimagefromdata?
  /// @todo This is currently undocumented because I have no AA or thick textures to work off of.
  if (num_samples > 1 && m_num_micro_tile_bytes > static_cast<quint64>(SPLIT_SIZE))
  {
    samples_per_slice = SPLIT_SIZE / m_bytes_per_sample;
    num_sample_splits = num_samples / samples_per_slice;
    // TODO: Could this be written directly to m_num_samples?
    num_samples = static_cast<quint32>(samples_per_slice);

    tile_slice_bits = m_num_micro_tile_bits / num_sample_splits;
    sample_slice = elem_offset / tile_slice_bits;
    elem_offset %= tile_slice_bits;
  }
  else
  {
    samples_per_slice = num_samples;
    num_sample_splits = 1;
    sample_slice = 0;
  }

  // This might have some correlation with pBitPosition?
  elem_offset /= 8;

  quint32 x_3 = Bit(x, 3);
  quint32 x_4 = Bit(x, 4);

  quint32 y_3 = Bit(y, 3);

  quint64 macro_tile_index_x = x / m_macro_tile_pitch;
  quint64 macro_tile_index_y = y / m_macro_tile_height;

  // The Wii U has 4 RAM chips, here we select a seemingly "random" one using an
  // algorithm to generate one from the coordinates.
  quint32 bank_bit_0 = Bit((y / (16 * NUM_PIPES)) ^ x_3);
  quint32 bank_bit_1 = Bit((y / (8 * NUM_PIPES)) ^ x_4);
  quint32 bank = MakeByte(bank_bit_0, bank_bit_1);

  // Each of the Wii U's RAM chips has 2 channels, here we select a seemingly
  // "random" one using an algorithm to generate one from the coordinates.
  quint32 pipe = Bit((y_3 ^ x_3));

  // Random bank index <<
  // Number of bank bits
  // Random bank index, shifted over in its correct spot.

  // Shifted bank index |
  // Random pipe index =
  // Three bits containing the random bank and pipe.
  quint64 bank_pipe = (bank << NUM_PIPE_BITS) | pipe;

  // Number of pipes *
  // Bank index specified by texture =
  // Bank index specified by texture, shifted over in its correct spot.

  // Shifted specified bank index +
  // Specified pipe index =
  // Three bits containing the bank and pipe specified by the texture.
  quint64 swizzle = (m_bank_swizzle << NUM_PIPE_BITS) | m_pipe_swizzle;

  // The current slice the pixel is in, for 3D textures.
  quint64 sliceIn = slice;

  if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
    sliceIn /= static_cast<quint32>(m_tile_mode_info.thickness);

  // Algorithm to recalculate bank and pipe?
  bank_pipe ^= (sample_slice * ((NUM_BANKS >> 1) + 1)) << NUM_PIPE_BITS ^
               (swizzle + sliceIn * m_common_tile_mode_rotation);
  bank_pipe %= NUM_PIPES * NUM_BANKS;
  pipe = bank_pipe % NUM_PIPES;
  bank = bank_pipe / NUM_PIPES;

  quint32 slice_offset = m_num_slice_bytes * ((sample_slice + num_sample_splits * slice) /
                                              static_cast<quint32>(m_tile_mode_info.thickness));

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
    quint64 bank_swap_width = ComputeSurfaceBankSwappedWidth(m_base_header.pitch);
    quint64 swap_index = m_macro_tile_pitch * macro_tile_index_x / bank_swap_width;
    quint64 bank_mask = NUM_BANKS - 1;
    bank ^= bankSwapOrder[swap_index & bank_mask];
  }
  // Calculate final offset
  // Get mask targeting every group bit.
  quint64 group_mask = InclusiveBitMask(NUM_LOW_OFFSET_BITS);
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
      elem_offset + ((macro_tile_offset + slice_offset) >> (NUM_BANK_BITS + NUM_PIPE_BITS));

  // Get the part of the pixel offset left of the pipe and bank.
  quint64 offset_high = (total_offset & ~group_mask) << (NUM_BANK_BITS + NUM_PIPE_BITS);
  // Get the part of the pixel offset right of the pipe and bank.
  quint64 offset_low = total_offset & group_mask;
  // Get the actual pipe and bank.
  quint64 bank_bits = bank << (NUM_PIPE_BITS + NUM_LOW_OFFSET_BITS);
  quint64 pipe_bits = pipe << NUM_LOW_OFFSET_BITS;
  // Put it all together.
  quint64 offset = bank_bits | pipe_bits | offset_low | offset_high;

#ifdef DEBUG
#ifdef VERBOSE
  qDebug() << "-----------------";
  qDebug("Bank (Bin):            %s", QString::number(bank, 2).toStdString().c_str());
  qDebug("Pipe (Bin):              %s", QString::number(pipe, 2).toStdString().c_str());
  qDebug("High Offset:     %s", QString::number(offset_high, 2).toStdString().c_str());
  qDebug("Low Offset:               %s", QString::number(offset_low, 2).toStdString().c_str());
#endif
  qDebug("Final Offset:    %s (AKA: 0x%s)", QString::number(offset, 2).toStdString().c_str(),
         QString::number(offset, 16).toStdString().c_str());
#endif

  return offset;
}

quint32 GX2ImageBase::GetPixelIndexMicroTiled(quint32 x, quint32 y, quint32 z)
{
  quint32 pixel_bit_0 = 0;
  quint32 pixel_bit_1 = 0;
  quint32 pixel_bit_2 = 0;
  quint32 pixel_bit_3 = 0;
  quint32 pixel_bit_4 = 0;
  quint32 pixel_bit_5 = 0;
  quint32 pixel_bit_6 = 0;
  quint32 pixel_bit_7 = 0;

  quint32 x_0 = Bit(x, 0);
  quint32 x_1 = Bit(x, 1);
  quint32 x_2 = Bit(x, 2);
  quint32 y_0 = Bit(y, 0);
  quint32 y_1 = Bit(y, 1);
  quint32 y_2 = Bit(y, 2);
  quint32 z_0 = Bit(z, 0);
  quint32 z_1 = Bit(z, 1);

  if (m_micro_tile_type != MicroTileType::Displayable)
  {
    pixel_bit_0 = x_0;
    pixel_bit_1 = y_0;
    pixel_bit_2 = x_1;
    pixel_bit_3 = y_1;
    pixel_bit_4 = x_2;
    pixel_bit_5 = y_2;
  }
  else
  {
    switch (m_common_format_info.bpp)
    {
    case 8:
      pixel_bit_0 = x_0;
      pixel_bit_1 = x_1;
      pixel_bit_2 = x_2;
      pixel_bit_3 = y_1;
      pixel_bit_4 = y_0;
      pixel_bit_5 = y_2;
      break;
    case 16:
      pixel_bit_0 = x_0;
      pixel_bit_1 = x_1;
      pixel_bit_2 = x_2;
      pixel_bit_3 = y_0;
      pixel_bit_4 = y_1;
      pixel_bit_5 = y_2;
      break;
    case 64:
      pixel_bit_0 = x_0;
      pixel_bit_1 = y_0;
      pixel_bit_2 = x_1;
      pixel_bit_3 = x_2;
      pixel_bit_4 = y_1;
      pixel_bit_5 = y_2;
      break;
    case 128:
      pixel_bit_0 = y_0;
      pixel_bit_1 = x_0;
      pixel_bit_2 = x_1;
      pixel_bit_3 = x_2;
      pixel_bit_4 = y_1;
      pixel_bit_5 = y_2;
      break;
    case 32:
    case 96:
    default:
      pixel_bit_0 = x_0;
      pixel_bit_1 = x_1;
      pixel_bit_2 = y_0;
      pixel_bit_3 = x_2;
      pixel_bit_4 = y_1;
      pixel_bit_5 = y_2;
      break;
    }
  }

  if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
  {
    pixel_bit_6 = z_0;
    pixel_bit_7 = z_1;
  }

  return MakeByte(pixel_bit_0, pixel_bit_1, pixel_bit_2, pixel_bit_3, pixel_bit_4, pixel_bit_5,
                  pixel_bit_6, pixel_bit_7);
}

quint32 GX2ImageBase::ComputeSurfaceBankSwappedWidth(quint32 pitch)
{
  quint32 bank_swap_width = 0;
  quint32 slices_per_tile = 1;

  quint32 num_samples = m_num_samples;
  // TODO: Same as m_bytes_per_sample?
  quint32 bytes_per_sample = 8 * m_common_format_info.bpp;
  quint32 samples_per_tile = SPLIT_SIZE / bytes_per_sample;

  if (SPLIT_SIZE / m_bytes_per_sample)
  {
    slices_per_tile = qMax<uint32_t>(1u, num_samples / samples_per_tile);
  }

  if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
    num_samples = 4;

  quint32 bytes_per_tile_slice = num_samples * m_bytes_per_sample / slices_per_tile;

  if (m_tile_mode_info.thickness == TileModeInfo::Thickness::Thick)
  {
    auto swapTiles = qMax<uint32_t>(1u, (SWAP_SIZE >> 1) / m_common_format_info.bpp);
    quint64 swapWidth = swapTiles * 8 * NUM_BANKS;
    auto heightBytes = num_samples * m_tile_mode_info.aspect_ratio * NUM_PIPES *
                       m_common_format_info.bpp / slices_per_tile;
    quint64 swapMax = NUM_PIPES * NUM_BANKS * ROW_SIZE / heightBytes;
    quint64 swapMin = PIPE_INTERLEAVE_BYTES * 8 * NUM_BANKS / bytes_per_tile_slice;

    bank_swap_width = qMin(swapMax, qMax(swapMin, swapWidth));

    while (bank_swap_width >= 2 * pitch)
    {
      bank_swap_width >>= 1;
    }
  }

  return bank_swap_width;
}
