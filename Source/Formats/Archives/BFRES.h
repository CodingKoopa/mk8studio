#pragma once

#include <QVector>

#include "Common.h"
#include "File.h"
#include "Formats/Common/ResourceDictionary.h"
#include "Formats/FormatBase.h"
#include "Formats/Models/FMDL.h"
#include "Formats/Textures/FTEX.h"

class BFRES : public FormatBase
{
public:
  BFRES(File* file = nullptr, quint32 start_offset = 0);
  BFRES(const BFRES& other);
  BFRES& operator=(const BFRES& other);

  // TODO (For C++17) Make a base structs for the info structs like this, or the ones in
  // GX2ImageBase, so the GetValueFromName or GetIndexOf functions can be there instead of redoing
  // it for every struct.
  struct EndianInfo
  {
    enum class Endianness
    {
      Little = 0xFFFE,
      Big = 0xFEFF
    } value;
    QString name;
  };

  struct Header
  {
    QString magic;
    quint8 unknown_a;
    quint8 unknown_b;
    quint8 unknown_c;
    quint8 unknown_d;
    EndianInfo endian_info;
    quint16 unknown_e;
    quint32 length;
    quint32 alignment;
    quint32 file_name_offset;
    quint32 string_table_length;
    quint32 string_table_offset;
    QVector<quint32> file_offsets;
    QVector<quint16> file_counts;
    quint32 unknown_f;

    QString file_name;
  };

  struct IndexGroupHeader
  {
    quint32 length;
    quint32 num_entries;
  };

  struct Node
  {
    quint32 search_value;
    quint16 left_index;
    quint16 right_index;
    quint32 name_ptr;
    quint32 data_ptr;

    QString name;
    Node* left_node = nullptr;
    Node* right_node = nullptr;
  };

  enum class GroupType
  {
    FMDL = 0,
    FTEX = 1
  };

  ResultCode ReadHeader();
  ResultCode ReadDictionaries();

  const Header& GetHeader() const;
  void SetHeader(const Header& header);

  File* GetFile() const;

  const ResourceDictionary<FMDL>& GetFMDLDictionary();
  void SetFMDLDictionary(const ResourceDictionary<FMDL>& dictionary);
  const ResourceDictionary<FTEX>& GetFTEXDictionary();
  void SetFTEXDictionary(const ResourceDictionary<FTEX>& dictionary);

  const QVector<EndianInfo>& GetEndianInfoList() const;
  const BFRES::EndianInfo& GetEndianInfoFromName(const QString& name);
  quint32 GetEndianIndexFromInfo(const EndianInfo& endian_info);

private:
  const QVector<EndianInfo> m_endian_info_list{{EndianInfo::Endianness::Little, "Little Endian"},
                                               {EndianInfo::Endianness::Big, "Big Endian"}};

  Header m_header;

  ResourceDictionary<FMDL> m_fmdl_dictionary;
  ResourceDictionary<FTEX> m_ftex_dictionary;

  const quint32 m_num_groups = 12;
};