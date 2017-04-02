#include "gx2.h"

#include "dds.h"

GX2::GX2()
{
}

GX2::~GX2()
{
  delete m_header;
}

ResultCode GX2::ReadImageFromData()
{
  ComputeSurfaceThickness();
  ComputeSurfaceRotationFromTileMode();
  ComputeThickMicroTiling();
  ComputeBankSwappedTileMode();

  quint32 width = m_header->width;
  quint32 height = m_header->height;

  m_num_samples = 1;

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
  // deswizzledImageData = QByteArray();
  deswizzledImageData.resize(m_header->data_length);
  deswizzledImageData.fill(0);

  for (quint32 y = 0; y < height; ++y)
  {
    for (quint32 x = 0; x < width; ++x)
    {
      qint32 origOffset;
      // Get the offset of the pixel at the current coordinate.
      switch (m_header->tile_mode)
      {
      case GX2_TILING_2D_TILED_THIN1:
      default:
        origOffset = ComputeSurfaceAddrFromCoordMacroTiled(
            x, y, 0, 0, 0, 0, 0, ((m_header->swizzle >> 8) & 1), ((m_header->swizzle >> 9) & 3), 0);
      }

      qint32 new_pos;

      // Write the new pixels in their normal order to the new byte array.
      switch (m_header->format)
      {
      case GX2_FMT_BC1_UNORM:
      case GX2_FMT_BC1_SRGB:
      case GX2_FMT_BC4_UNORM:
      case GX2_FMT_BC4_SNORM:
      {
        new_pos = (y * width + x) * 8;
        break;
      }
      default:
        new_pos = (y * width + x) * 16;
        break;
      }

      // TODO: there's probably a better way of approaching this
      for (int i = 0; i < 8; i++)
      {
        if (origOffset + i > raw_image_data.size())
        {
          qDebug("Error: Tried to read pixel outside of image data. "
                 "Skipping pixel.");
          continue;
        }
        if (new_pos + i > deswizzledImageData.size())
        {
          qDebug() << "Error: Tried to write pixel outside of image data. "
                      "Skipping pixel.";
          continue;
        }
        // qDebug("Writing raw offset %04X to %04X...", origOffset + i, newPos + i);
        deswizzledImageData[new_pos + i] = raw_image_data[origOffset + i];
        // qDebug("Deswizzled data using at(): %0X.",
        // rawImageData.at(newPos + i));
        // qDebug("Size: %i", deswizzledImageData.size());
        //            qDebug("Done.");
      }
    }
  }
  DDS dds(&deswizzledImageData);
  dds.MakeHeader(m_header->width, m_header->height, m_header->depth, m_header->num_mips, true,
                 m_header->format);
  dds.WriteFile(m_name);
  return RESULT_SUCCESS;
}

quint64 GX2::ComputeSurfaceAddrFromCoordMacroTiled(quint32 x, quint32 y, quint32 slice,
                                                   quint32 sample, bool isDepth, quint32 tileBase,
                                                   quint32 compBits, quint32 pipeSwizzle,
                                                   quint32 bankSwizzle, quint32* pBitPosition)
{
  quint64 numPipes = m_pipes;
  quint64 numBanks = m_banks;
  quint64 numGroupBits = m_group_bit_count;
  quint64 numPipeBits = m_pipe_bit_count;
  quint64 numBankBits = m_bank_bit_count;

  // The number of pixels in each micro tile * The thickness of each micro tile
  // = Number of pixels in each micro tile with thickness taken In into account.
  // Recalculated number of pixels in each micro tile * The number of bits in
  // one pixel
  // = Number of bits in one sample of a micro tile.
  // Number of bits in one sample of a micro tile * The number of samples
  // = The number of bits in one micro tile, including all samples.
  quint64 microTileBits = MicroTilePixels * m_micro_tile_thickness * m_bpp * m_num_samples;
  // Convert the number of bits to bytes.
  quint64 microTileBytes = microTileBits / 8;

  // Get the pixel index within the micro tile.
  quint64 pixelIndex = ComputePixelIndexWithinMicroTile(x, y, slice, GetTileType(isDepth));

  // The offset of the beginning of the micro tiles in the current sample.
  quint64 sampleOffset;
  // The offset of the current pixel relative to the beginning of the current
  // sample.
  quint64 pixelOffset;

  if (isDepth)
  {
    if (compBits && compBits != m_bpp)
    {
      sampleOffset = tileBase + compBits * sample;
      pixelOffset = m_num_samples * compBits * pixelIndex;
    }
    else
    {
      sampleOffset = m_bpp * sample;
      pixelOffset = m_num_samples * m_bpp * pixelIndex;
    }
  }
  else
  {
    // The number of bits in one micro tile / The number of samples = The number
    // of bits in one micro tile in one sample.
    // The number of bits in one micro tile in one sample * The current sample
    // we're in = The offset of the start of the micro tiles for our sample.
    sampleOffset = sample * (microTileBits / m_num_samples);
    //    qDebug("Sample Offset: %04llX", sampleOffset);
    // The number of bits in one pixel * The pixel index = The offset of the
    // current pixel relative to the beginning of the current sample.
    pixelOffset = m_bpp * pixelIndex;
    //    qDebug("Pixel Offset: %04llX", pixelOffset);
  }

  // The offset of the pixel + The offset of the beginning of the current sample
  // = The recalculated pixel offset with sampling in mind.
  quint64 elemOffset = pixelOffset + sampleOffset;
  // qDebug("Elem Offset: %04llX", elemOffset);

  // ???
  if (pBitPosition != nullptr)
    *pBitPosition = static_cast<quint32>(elemOffset % 8);

  // The number of bytes in one micro tile, including all samples / How many
  // samples there are = The number of bytes in one micro tile in one sample.
  quint64 bytesPerSample = microTileBytes / m_num_samples;
  // How many samples there are in each slice (/ layer?)
  quint64 samplesPerSlice;
  // ???
  quint64 numSampleSplits;
  // Which slice the sample lies in?
  quint64 sampleSlice;
  quint64 tileSliceBits;

  // If there's more than one sample, and ???
  if (m_num_samples > 1 && microTileBytes > static_cast<quint64>(m_split_size))
  {
    // The size of one split / The number of bytes in one micro tile in one
    // sample = The number of samples (Micro tiles) that can fit in each slice.
    samplesPerSlice = m_split_size / bytesPerSample;
    // The number of samples / The number of samples that can fit in each slice
    // = The number of splits (Dunno what a split is, maybe the boundary between
    // two slices?)
    numSampleSplits = m_num_samples / samplesPerSlice;
    m_num_samples = static_cast<quint32>(samplesPerSlice);

    // The number of bits in one micro tile / the number of sample splits = ?
    tileSliceBits = microTileBits / numSampleSplits;
    // The recalculated pixel offset / tileSliceBits = ?
    sampleSlice = elemOffset / tileSliceBits;
    elemOffset %= tileSliceBits;
  }
  else
  {
    // How many samples can fit in one slice, here it can just be 1 (Or 0,
    // depending on m_num_samples.).
    samplesPerSlice = m_num_samples;
    // The number of splits?
    numSampleSplits = 1;
    // ???
    sampleSlice = 0;
  }

  // This might have some correlation with pBitPosition
  elemOffset /= 8;

  // Each of the Wii U's RAM chips has 2 pipes, here we select a seemingly
  // "random" one using an algorithm to generate one from the coordinates.
  quint64 pipe = ComputePipeFromCoordWoRotation(x, y);
  // The Wii U has 4 RAM chips, here we select a seemingly "random" one using an
  // algorithm to generate one from the coordinates.
  quint64 bank = ComputeBankFromCoordWoRotation(x, y);
  //  qDebug() << "X: " << x << "Y: " << y << "Bank: " << bank;
  //  qDebug() << "X: " << x << "Y: " << y << "Pipe: " << pipe;

  // The maximum number of pipes * The random bank index = ?
  // The product + The random pipe index = The bank and pipe together in a
  // variable, here no larger than 7.
  quint64 bankPipe = pipe + numPipes * bank;
  // ?
  quint64 swizzle = pipeSwizzle + numPipes * bankSwizzle;
  // The current slice the pixel is in.
  quint64 sliceIn = slice;

  if (thickMacroTiled)
    sliceIn /= ThickTileThickness;

  // Algorithm to recalculate bank and pipe?
  bankPipe ^= numPipes * sampleSlice * ((numBanks >> 1) + 1) ^ (swizzle + sliceIn * rotate);
  bankPipe %= numPipes * numBanks;
  pipe = bankPipe % numPipes;
  bank = bankPipe / numPipes;

  ComputeSliceInfo(slice, sampleSlice, numSampleSplits);

  quint64 macroTilePitch = 8 * m_banks;
  quint64 macroTileHeight = 8 * m_pipes;

  switch (m_header->tile_mode)
  {
  case GX2_TILING_2D_TILED_THIN2:
  case GX2_TILING_2B_TILED_THIN2:
    macroTilePitch /= 2;
    macroTileHeight *= 2;
    break;
  case GX2_TILING_2D_TILED_THIN4:
  case GX2_TILING_2B_TILED_THIN4:
    macroTilePitch /= 4;
    macroTileHeight *= 4;
    break;
  default:
    break;
  }

  quint64 macroTilesPerRow = m_header->pitch / macroTilePitch;
  quint64 macroTileBytes = BITS_TO_BYTES(m_num_samples * m_micro_tile_thickness * m_bpp *
                                         macroTileHeight * macroTilePitch);
  quint64 macroTileIndexX = x / macroTilePitch;
  quint64 macroTileIndexY = y / macroTileHeight;
  quint64 macroTileOffset = macroTileBytes * (macroTileIndexX + macroTilesPerRow * macroTileIndexY);

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
    quint64 swap_index = macroTilePitch * macroTileIndexX / bank_swap_width;
    quint64 bank_mask = m_banks - 1;
    bank ^= bankSwapOrder[swap_index & bank_mask];
    break;
  }
  default:
    break;
  }

  // Calculate final offset
  // Get mask targeting every group bit.
  quint64 group_mask = (1 << numGroupBits) - 1;
  quint64 total_offset =
      elemOffset + ((macroTileOffset + m_slice_offset) >> (numBankBits + numPipeBits));

  quint64 offset_high = (total_offset & ~group_mask) << (numBankBits + numPipeBits);
  quint64 offset_low = total_offset & group_mask;
  quint64 bankBits = bank << (numPipeBits + numGroupBits);
  quint64 pipeBits = pipe << numGroupBits;
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

quint32 GX2::ComputePixelIndexWithinMicroTile(quint32 x, quint32 y, quint32 z,
                                              gx2MicroTileType_t tileType)
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

  if (tileType == GX2_MICRO_TILING_THICK_TILING)
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
    if (tileType != GX2_MICRO_TILING_DISPLAYABLE)
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

GX2::gx2MicroTileType_t GX2::GetTileType(bool isDepth)
{
  if (isDepth)
  {
    return GX2_MICRO_TILING_DISPLAYABLE;
  }
  else
  {
    return GX2_MICRO_TILING_DISPLAYABLE;
  }
}

void GX2::ComputeSurfaceThickness()
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

void GX2::ComputeSurfaceRotationFromTileMode()
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

void GX2::ComputeThickMicroTiling()
{
  switch (m_header->tile_mode)
  {
  case GX2_TILING_2D_TILED_THICK:
  case GX2_TILING_2B_TILED_THICK:
  case GX2_TILING_3D_TILED_THICK:
  case GX2_TILING_3B_TILED_THICK:
    thickMacroTiled = true;
  default:
    thickMacroTiled = false;
  }
}

void GX2::ComputeBankSwappedTileMode()
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

void GX2::ComputeSliceInfo(quint32 slice, quint32 sampleSlice, quint32 numSampleSplits)
{
  m_slice_bytes = BITS_TO_BYTES(m_header->pitch * m_header->height * m_micro_tile_thickness *
                                m_bpp * m_num_samples);
  m_slice_offset =
      m_slice_bytes * ((sampleSlice + numSampleSplits * slice) / m_micro_tile_thickness);
}

quint32 GX2::ComputeMacroTileAspectRatio()
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

quint32 GX2::ComputeSurfaceBankSwappedWidth(quint32 pitch, quint32* pSlicesPerTile)
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

  if (thickMacroTiled)
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

quint32 GX2::ComputePipeFromCoordWoRotation(quint32 x, quint32 y)
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

quint32 GX2::ComputeBankFromCoordWoRotation(quint32 x, quint32 y)
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

void GX2::SetName(const QString& value)
{
  m_name = value;
}
