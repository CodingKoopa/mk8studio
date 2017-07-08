#include "File.h"

File::File(const QString& path)
{
  m_file = new QFile(path);
  m_stream = new QDataStream(m_file);
  if (!m_file->open(QIODevice::ReadWrite))
  {
    if (!m_file->open(QIODevice::ReadOnly))
      m_can_read = false;
    else
      m_read_only = true;
  }
}

File::~File()
{
  if (m_file)
  {
    if (m_file->isOpen())
      m_file->close();
    delete m_file;
  }
  if (m_stream)
    delete m_stream;
}

quint64 File::Pos()
{
  return m_file->pos();
}

bool File::Seek(quint64 pos)
{
  return m_file->seek(pos);
}

void File::Skip(qint64 num)
{
  Seek(Pos() + num);
}

quint64 File::Size()
{
  return m_file->size();
}

void File::Save()
{
  m_file->flush();
}

bool File::GetReadOnly()
{
  return m_read_only;
}

bool File::GetCanRead()
{
  return m_can_read;
}

quint8 File::Read8()
{
  quint8 ret;
  *m_stream >> ret;
  return ret;
}

quint16 File::Read16()
{
  quint16 ret;
  *m_stream >> ret;
  return ret;
}

quint32 File::ReadU32()
{
  quint32 ret;
  *m_stream >> ret;
  return ret;
}

qint32 File::ReadS32()
{
  qint32 ret;
  *m_stream >> ret;
  return ret;
}

quint32 File::ReadU32RelativeOffset()
{
  quint32 pos = Pos();
  quint32 ret = ReadU32();
  if (!ret)
    return 0;
  else
    return pos + ret;
}

qint32 File::ReadS32RelativeOffset()
{
  quint64 pos = Pos();
  qint32 ret = ReadS32();
  if (!ret)
    return 0;
  else
    return pos + ret;
}

void File::ReadBytes(quint32 len, char* buffer)
{
  m_stream->readRawData(buffer, len);
}

QString File::ReadStringASCII(quint32 len)  // len=0 for NULL terminated string
{
  QString ret;
  char temp_8[64];
  QChar temp_16[64];

  if (!len)
    len = 0xFFFFFFFF;  // lazy

  bool terminated = false;
  quint32 len_read = 0;
  while (len_read < len && !terminated)
  {
    quint32 this_len = (len_read + 64 > len) ? len : len_read + 64;
    m_file->read((char*)temp_8, this_len);

    quint32 actual_len = 0;
    for (quint32 i = 0; i < this_len; i++)
    {
      temp_16[i] = (QChar)temp_8[i];
      if (!temp_8[i])
      {
        terminated = true;
        break;
      }
      ++actual_len;
    }

    ret.append(temp_16, actual_len);
    len_read += actual_len;
  }

  return ret;
}

void File::Write8(quint8 val)
{
  *m_stream << val;
}

void File::Write16(quint16 val)
{
  *m_stream << val;
}

void File::Write32(quint32 val)
{
  *m_stream << val;
}

void File::Write32RelativeOffset(quint32 val)
{
  Write32(val - Pos());
}

int File::WriteBytes(char* data, quint32 len)
{
  return m_stream->writeRawData(data, len);
}

void File::WriteStringASCII(QString str, int len)
{
  if (len < str.length())
    str = str.left(len);

  for (int i = 0; i < str.length(); ++i)
    Write8(str.at(i).toLatin1());

  for (int i = 0; i < len - str.length(); ++i)
    Write8(0);
}

void File::SetByteOrder(QDataStream::ByteOrder order)
{
  m_stream->setByteOrder(order);
}
