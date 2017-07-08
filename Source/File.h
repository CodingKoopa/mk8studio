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

#pragma once

#include <QDataStream>
#include <QFile>
#include <QString>

#include "NonCopyable.h"

class File : NonCopyable
{
public:
  File(const QString& path);
  ~File();

  bool GetReadOnly();
  bool GetCanRead();

  quint8 Read8();
  quint16 Read16();
  quint32 ReadU32();
  qint32 ReadS32();
  quint32 ReadU32RelativeOffset();
  qint32 ReadS32RelativeOffset();
  void ReadBytes(quint32 len, char* buffer);
  QString ReadStringASCII(quint32 len = 0);

  void Write8(quint8 val);
  void Write16(quint16 val);
  void Write32(quint32 val);
  void Write32RelativeOffset(quint32 val);
  int WriteBytes(char* data, quint32 len);
  void WriteStringASCII(QString str, int len = 0);

  void SetByteOrder(QDataStream::ByteOrder order);

  quint64 Pos();
  bool Seek(quint64 pos);
  void Skip(qint64 num);
  quint64 Size();

  void Save();

protected:
  QFile* m_file;
  QDataStream* m_stream;
  bool m_read_only = false;
  bool m_can_read = true;
};
