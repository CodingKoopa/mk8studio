#include "filebase.h"

#include <QDebug>

FileBase::FileBase(QString path) : m_file(new QFile(path)), m_stream(m_file)
{
  if (!m_file->open(QIODevice::ReadWrite))
  {
    if (!m_file->open(QIODevice::ReadOnly))
      canRead = false;
    else
      readOnly = true;
  }
}

FileBase::~FileBase()
{
  m_file->close();
  delete m_file;
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
  return readOnly;
}

bool FileBase::GetCanRead()
{
  return canRead;
}

quint8 FileBase::Read8()
{
  quint8 ret;
  m_stream >> ret;
  return ret;
}

quint16 FileBase::Read16()
{
  quint16 ret;
  m_stream >> ret;
  return ret;
}

quint32 FileBase::Read32()
{
  quint32 ret;
  m_stream >> ret;
  return ret;
}

char* FileBase::ReadBytes(quint32 len)
{
  char* buffer = new char[len];

  m_stream.readRawData(buffer, len);

  return buffer;
}

QString FileBase::ReadStringASCII(quint32 len)  // len=0 for NULL terminated string
{
  QString ret;
  char temp8[64];
  QChar temp16[64];

  if (!len)
    len = 0xFFFFFFFF;  // lazy

  bool terminated = false;
  quint32 lenread = 0;
  while (lenread < len && !terminated)
  {
    quint32 thislen = (lenread + 64 > len) ? len : lenread + 64;
    m_file->read((char*)temp8, thislen);

    quint32 actuallen = 0;
    for (quint32 i = 0; i < thislen; i++)
    {
      temp16[i] = (QChar)temp8[i];
      if (!temp8[i])
      {
        terminated = true;
        break;
      }
      actuallen++;
    }

    ret.append(temp16, actuallen);
    lenread += actuallen;
  }

  return ret;
}

void FileBase::Write8(quint8 val)
{
  m_stream << val;
}

void FileBase::Write16(quint16 val)
{
  m_stream << val;
}

void FileBase::Write32(quint32 val)
{
  m_stream << val;
}

int FileBase::WriteBytes(char* data, quint32 len)
{
  return m_stream.writeRawData(data, len);
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
  m_stream.setByteOrder(order);
}
