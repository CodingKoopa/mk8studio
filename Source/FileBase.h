/*
    Copyright 2015 StapleButter

    This file is part of CoinKiller.

    CoinKiller is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    CoinKiller is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along
    with CoinKiller. If not, see http://www.gnu.org/licenses/.
*/

#ifndef FILEBASE
#define FILEBASE

#include <QDataStream>
#include <QFile>
#include <QString>

class FileBase
{
public:
  FileBase(QString path);
  ~FileBase();

  bool GetReadOnly();
  bool GetCanRead();

  quint8 Read8();
  quint16 Read16();
  quint32 Read32();
  quint32 Read32RelativeOffset();
  char* ReadBytes(quint32 len);
  QString ReadStringASCII(quint32 len = 0);

  void Write8(quint8 val);
  void Write16(quint16 val);
  void Write32(quint32 val);
  void Write32RelativeOffset(quint32 val);
  int WriteBytes(char* data, quint32 len);
  void WriteStringASCII(QString str, int len = 0);

  void SetByteOrder(QDataStream::ByteOrder order);

  quint64 Pos();
  bool Seek(quint64 Pos);
  void Skip(qint64 num);
  quint64 Size();

  void Save();

protected:
  QFile* m_file;
  QDataStream m_stream;
  bool readOnly = false;
  bool canRead = true;
};

#endif  // FILEBASE
