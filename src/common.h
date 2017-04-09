#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

#define BITS_PER_BYTE 8
// TODO: cleaner solution using c++ features
#define BITS_TO_BYTES(x) (((x) + (BITS_PER_BYTE - 1)) / BITS_PER_BYTE)
#define BYTES_TO_BITS(x) ((x)*BITS_PER_BYTE)
#define GET_BIT(v, b) (((v) >> (b)) & 1)

enum ResultCode
{
  RESULT_SUCCESS,
  // For noncritical updates on things
  RESULT_STATUS_BAR_UPDATE,
  RESULT_BFRES_HEADER_SIZE_ERROR,
  RESULT_BFRES_ENDIANNESS,
  RESULT_UNSUPPORTED_FILE_FORMAT
};

#endif  // COMMON_H
