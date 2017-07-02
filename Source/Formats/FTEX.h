#pragma once

#include <QString>

#include "FileBase.h"
#include "GX2ImageBase.h"

class FTEX : public GX2ImageBase
{
public:
  FTEX(FileBase* file, quint64 pos);
  FTEX(const FTEX& other);
  FTEX& operator=(const FTEX& other);
  ~FTEX();

  struct FTEXHeader : ImageHeaderBase
  {
    QString magic;
    quint32 dim;
    quint32 usage;
    quint32 mipmap_length;
    quint32 alignment;
    quint32 data_offset;
    quint32 mipmap_offset;
  };

  ResultCode ReadHeader();
  ResultCode ReadImage();

  void InjectImage();

  quint64 GetStart();

  const FTEXHeader& GetHeader();
  void SetHeader(const FTEXHeader& ftex_header);

private:
  void DeepCopyRawImageDataBuffer(const FTEX& other);

  FileBase* m_file;
  quint64 m_start_offset;
  FTEXHeader m_header;
  char* m_raw_image_data_buffer = nullptr;
};
