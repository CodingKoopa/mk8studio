#ifndef FTEX_H
#define FTEX_H

#include <QString>

#include "FileBase.h"
#include "GX2ImageBase.h"

class FTEX : public GX2ImageBase
{
public:
  FTEX(FileBase* file, quint64 pos);
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

  QImage* GetImage();

  quint64 GetStart();

  FTEXHeader* GetHeader() { return m_header; }

  void SetHeader(FTEXHeader* ftex_header) { m_header = ftex_header; }

private:
  FileBase* m_file;
  quint64 m_start_offset;
  FTEXHeader* m_header = nullptr;
  char* m_raw_image_data_buffer = nullptr;
};

#endif  // FTEX_H
