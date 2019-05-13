#pragma once

#include <QDebug>

/// Gets the size of a type, in bits.
///
/// @tparam T  The type to get the size of.
///
/// @return The size of the type, in bits.
template <typename T>
constexpr size_t GetBitSize() noexcept
{
  return sizeof(T) * CHAR_BIT;
}

/// Gets the bit at a specified position of a byte.
///
/// @tparam T1        The type of the byte, and bit.
///
/// @param  byte      The byte to operate on.
/// @param  position  The position to extract the bit from. If omitted, the LSB will be used.
///
/// @return The extracted bit.
template <typename T>
constexpr T GetBit(const T byte, const size_t position = 0) noexcept
{
  return (byte >> position) & static_cast<T>(1);
}

/// Gets the bits in a specified range of a byte. This is inclusive.
///
/// @tparam T       The type of the byte.
/// @tparam Result  The type of the bit. If omitted, the unsigned analog of T will be used.
///
/// @param  byte    The byte to operate on.
/// @param  begin   The beginning of the range.
/// @param  end     The end of the range.
///
/// @return The extracted bits.
template <typename T, typename Result = std::make_unsigned_t<T>>
constexpr Result GetBits(const T byte, const size_t begin, const size_t end) noexcept
{
  return static_cast<Result>(((static_cast<Result>(byte) << ((GetBitSize<T>() - 1) - end)) >>
                              (GetBitSize<T>() - end + begin - 1)));
}

/// Makes a byte from a specified set of bits.
///
/// @tparam T1    The type of the first bit.
/// @tparam T2    The type of the parameter pack of the rest of the bits.
///
/// @param  bit   The first bit.
/// @param  bits  The parameter back of the rest of the bits.
///
/// @return The new concatenated byte.
template <typename T1, typename... T2>
constexpr T1 MakeByte(T1 bit, T2... bits) noexcept
{
  T1 byte = bit;
  T1 pos = 0;
  ((byte |= ((bits & static_cast<T1>(1)) << ++pos)), ...);
  return byte;
}

/// Converts a number of bits to a number of bytes.
///
/// @param  num_bits  The number of bits.
///
/// @return The number of bytes.
constexpr size_t BitsToBytes(size_t num_bits) noexcept
{
  return num_bits / 8;
}

/// Converts a number of bytes to a number of bits.
///
/// @param  num_bytes The number of bytes.
///
/// @return The number of bits.
constexpr size_t BytesToBits(size_t num_bytes) noexcept
{
  return num_bytes * 8;
}

/// The possible results functions can return to let the caller know whether the action succeeded.
///
/// @todo This system for determining success isn't great. At the very least, there should be more
/// organization, and a consistent naming scheme.
enum class ResultCode
{
  Success,
  UpdateStatusBar,
  NotAvailable,
  FileNotFound,
  NoBytesWritten,
  IncorrectHeaderSize,
  IncorrectBFRESEndianness,
  UnsupportedFTEXComponent,
  UnsupportedTextureFormat,
  UnsupportedAttributeFormat,
  ImportantUnsupportedFileFormat
};
