#ifndef FTEX_H
#define FTEX_H

#include <QString>

#include "filebase.h"
#include "gx2.h"

class FTEX : public GX2
{
public:
  FTEX(FileBase* file, quint64 pos);
  ~FTEX();

  struct FTEXHeader : ImageHeader
  {
    QString magic;
    quint32 dim;
    // Width is inherited
    // Height is inherited
    // Depth is inherited
    // Number of mips is inherited
    quint32 aaMode;
    quint32 usage;
    // Deta length is inherited
    quint32 mipSize;
    // Swizzle is inherited
    quint32 alignment;
    // Pitch is inherited
    quint32 data_offset;
    quint32 mipmapOffset;
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
