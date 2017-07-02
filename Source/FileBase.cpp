#include "FileBase.h"

#include <QDebug>

FileBase::FileBase(const QString& path)
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

FileBase::~FileBase()
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

quint64 FileBase::Pos()
{
  return m_file->pos();
}

bool FileBase::Seek(quint64 pos)
{
  return m_file->seek(pos);
}

void FileBase::Skip(qint64 num)
{
  Seek(Pos() + num);
}

quint64 FileBase::Size()
{
  return m_file->size();
}

void FileBase::Save()
{
  m_file->flush();
}

bool FileBase::GetReadOnly()
{
  return m_read_only;
}

bool FileBase::GetCanRead()
{
  return m_can_read;
}

quint8 FileBase::Read8()
{
  quint8 ret;
  *m_stream >> ret;
  return ret;
}

quint16 FileBase::Read16()
{
  quint16 ret;
  *m_stream >> ret;
  return ret;
}

quint32 FileBase::Read32()
{
  quint32 ret;
  *m_stream >> ret;
  return ret;
}

quint32 FileBase::Read32RelativeOffset()
{
  quint32 pos = Pos();
  quint32 ret = Read32();
  if (!ret)
    return 0;
  else
    return pos + ret;
}

void FileBase::ReadBytes(quint32 len, char* buffer)
{
  m_stream->readRawData(buffer, len);
}

QString FileBase::ReadStringASCII(quint32 len)  // len=0 for NULL terminated string
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
      actual_len++;
    }

    ret.append(temp_16, actual_len);
    len_read += actual_len;
  }

  return ret;
}

void FileBase::Write8(quint8 val)
{
  *m_stream << val;
}

void FileBase::Write16(quint16 val)
{
  *m_stream << val;
}

void FileBase::Write32(quint32 val)
{
  *m_stream << val;
}

void FileBase::Write32RelativeOffset(quint32 val)
{
  Write32(val - Pos());
}

int FileBase::WriteBytes(char* data, quint32 len)
{
  return m_stream->writeRawData(data, len);
}

void FileBase::WriteStringASCII(QString str, int len)
{
  if (len < str.length())
    str = str.left(len);

  for (int i = 0; i < str.length(); i++)
    Write8(str.at(i).toLatin1());

  for (int i = 0; i < len - str.length(); i++)
    Write8(0);
}

void FileBase::SetByteOrder(QDataStream::ByteOrder order)
{
  m_stream->setByteOrder(order);
}
