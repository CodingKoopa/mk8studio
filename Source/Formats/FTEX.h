#ifndef FTEX_H
#define FTEX_H

#include <QString>

#include "FileBase.h"
#include "GX2ImageBase.h"

class FTEX : public GX2ImageBase
{
public:
  FTEX(FileBase* file, quint64 pos) : file(file), start(pos), m_ftex_header(nullptr) {}
  ~FTEX()
  {
    if (m_ftex_header)
      delete m_ftex_header;
  }

  struct FTEXHeader : ImageHeader
  {
    QString magic;
    quint32 dim;
    quint32 usage;
    quint32 mipSize;
    quint32 alignment;
    quint32 data_offset;
    quint32 mipmap_offset;
  };

  int ReadHeader();
  ResultCode ReadImageData();

  QImage* GetImage();

  FTEXHeader GetHeader();

  quint64 GetStart();

private:
  FileBase* file;
  quint64 start;
  FTEXHeader* m_ftex_header;
  QImage* image;
};

#endif  // FTEX_H
