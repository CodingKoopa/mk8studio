#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

template <typename T1>
T1 Bit(T1 byte)
{
  return byte & 1;
}

template <typename T1, typename T2>
T1 Bit(T1 byte, T2 position = 0)
{
  return (byte >> position) & 1;
}

template <typename T1>
T1 BitMask(T1 byte)
{
  return (1 << byte) - 1;
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
  return current_bit << position | MakeByteRecursive(position + 1, queue...);
}

template <typename T1, typename... T2>
T1 MakeByte(T1 current_bit, T2... queue)
{
  T1 byte = MakeByteRecursive(0, current_bit, queue...);
  return byte;
}

template <typename T>
T BitsToBytes(T num_bits)
{
  return num_bits / 8;
}

template <typename T>
T BytesToBits(T num_bytes)
{
  return num_bytes * 8;
}

// TODO: Switch to strongly typed enum class.
enum ResultCode
{
  RESULT_SUCCESS,
  // For noncritical updates on things
  RESULT_STATUS_BAR_UPDATE,
  RESULT_FILE_NOT_FOUND,
  RESULT_NO_BYTES_WRITTEN,
  RESULT_BFRES_HEADER_SIZE_ERROR,
  RESULT_BFRES_ENDIANNESS,
  RESULT_UNSUPPORTED_FILE_FORMAT,
  RESULT_UNSUPPORTED_FILE_FORMAT_IMPORTANT
};

#endif  // COMMON_H
