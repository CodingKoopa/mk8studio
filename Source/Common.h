#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

// TODO: constexpr?
template <typename T1>
T1 Bit(T1 byte)
{
  return byte & 1;
}

template <typename T1, typename T2>
T1 Bit(T1 byte, T2 position = 0)
{
  return (byte >> position) & static_cast<T1>(1);
}

template <typename T1>
T1 InclusiveBitMask(T1 byte)
{
  return (static_cast<T1>(1) << byte) - static_cast<T1>(1);
}

template <typename T1, typename T2, typename T3>
T1 GetBits(T1 byte, T2 position, T3 num_bits)
{
  return (byte >> position) & num_bits;
}

template <typename T1, typename T2>
T1 MakeByteRecursive(T1 position, T2 bit)
{
  return bit << position;
}
template <typename T1, typename T2, typename... T3>
T1 MakeByteRecursive(T1 position, T2 current_bit, T3... queue)
{
  return current_bit << position | MakeByteRecursive(position + static_cast<T1>(1), queue...);
}

template <typename T1, typename... T2>
T1 MakeByte(T1 current_bit, T2... queue)
{
  T1 byte = MakeByteRecursive(static_cast<T1>(0), current_bit, queue...);
  return byte;
}

template <typename T>
T BitsToBytes(T num_bits)
{
  return num_bits / static_cast<T>(8);
}

template <typename T>
T BytesToBits(T num_bytes)
{
  return num_bytes * static_cast<T>(8);
}

// TODO: Catagorize these in a different way?
enum class ResultCode
{
  Success,
  // For noncritical updates on things
  UpdateStatusBar,
  FileNotFound,
  NoBytesWritten,
  IncorrectHeaderSize,
  IncorrectBFRESEndianness,
  UnsupportedFileFormat,
  ImportantUnsupportedFileFormat
};

#endif  // COMMON_H
