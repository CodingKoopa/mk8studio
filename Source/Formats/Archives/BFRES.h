#pragma once

#include <memory>

#include "Common.h"
#include "File.h"
#include "Formats/Common/ResourceDictionary.h"
#include "Formats/FormatBase.h"
#include "Formats/Models/FMDL.h"
#include "Formats/Textures/FTEX.h"

class BFRES : public FormatBase
{
public:
  /// Initializes a new instance of the BFRES class.
  BFRES(std::shared_ptr<File> file, quint32 start_offset = 0);

  enum class Endianness
  {
    Little = 0xFFFE,
    Big = 0xFEFF
  } value;

  struct Header
  {
    QString magic;
    quint8 unknown_a;
    quint8 unknown_b;
    quint8 unknown_c;
    quint8 unknown_d;
    quint16 bom;
    quint16 unknown_e;
    quint32 length;
    quint32 alignment;
    quint32 file_name_offset;
    quint32 string_table_length;
    quint32 string_table_offset;
    QVector<quint32> file_offsets;
    QVector<quint16> file_counts;
    quint32 unknown_f;
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

  const ResourceDictionary<FMDL>& GetFMDLDictionary() const;
  void SetFMDLDictionary(const ResourceDictionary<FMDL>& dictionary);
  const ResourceDictionary<FTEX>& GetFTEXDictionary() const;
  void SetFTEXDictionary(const ResourceDictionary<FTEX>& dictionary);

  static std::map<Endianness, QString> GetEndianNames();
  const QString& GetEndianName() const;

private:
  static constexpr quint32 HEADER_SIZE = 0x6C;
  static constexpr quint32 NUM_GROUPS = 12;

  inline static const std::map<Endianness, QString> m_endian_names{
      {Endianness::Little, "Little Endian"}, {Endianness::Big, "Big Endian"}};
  QString m_endian_name;

  Header m_header;

  ResourceDictionary<FMDL> m_fmdl_dictionary;
  ResourceDictionary<FTEX> m_ftex_dictionary;
};
