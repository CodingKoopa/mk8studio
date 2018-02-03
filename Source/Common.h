#pragma once

#include <QDebug>

template <typename T1>
constexpr T1 Bit(T1 byte)
{
  return byte & 1;
}

template <typename T1, typename T2>
constexpr T1 Bit(T1 byte, T2 position = 0)
{
  return (byte >> position) & static_cast<T1>(1);
}

template <typename T1>
constexpr T1 InclusiveBitMask(T1 byte)
{
  return (static_cast<T1>(1) << byte) - static_cast<T1>(1);
}

template <typename T1, typename T2, typename T3>
constexpr T1 GetBits(T1 byte, T2 position, T3 num_bits)
{
  return (byte >> position) & num_bits;
}

template <typename T1, typename... T2>
constexpr T1 MakeByte(T1 bit, T2... bits)
{
  T1 byte = bit;
  quint32 pos = 0;
  ((byte |= ((bits & 1) << ++pos)), ...);
  return byte;
}

template <typename T>
constexpr T BitsToBytes(T num_bits)
{
  return num_bits / static_cast<T>(8);
}

template <typename T>
constexpr T BytesToBits(T num_bytes)
{
  return num_bytes * static_cast<T>(8);
}

// TODO: Catagorize these in a different way?
enum class ResultCode
{
  Success,
  // For noncritical updates on things
  UpdateStatusBar,
  NotAvailable,
  FileNotFound,
  NoBytesWritten,
  IncorrectHeaderSize,
  IncorrectBFRESEndianness,
  UnsupportedTextureFormat,
  UnsupportedAttributeFormat,
  ImportantUnsupportedFileFormat
};
