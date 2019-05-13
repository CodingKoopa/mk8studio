#pragma once

#include <memory>

#include "Common.h"
#include "File.h"
#include "Formats/Common/ResourceDictionary.h"
#include "Formats/FormatBase.h"
#include "Formats/Models/FMDL.h"
#include "Formats/Textures/FTEX.h"

/// @brief Represents a <b>b</b>inary ca<b>f</b>e <b>res</b>ource (%BFRES) graphics data archive.
///
/// %BFRES files contain pretty much everything you could ever want to know about a given grahical
/// resource. For example, for a given 3D character, the %BFRES for them would contain their
/// model(s), texture(s), animation(s), and more.
///
/// @todo This class does not yet take the string table into account.
class BFRES : public FormatBase
{
public:
  /// Initializes a new instance of the BFRES class.
  ///
  /// @param  file          Shared pointer to the file to read from.
  /// @param  start_offset  The offset to start reading at, where the BFRES structure starts.
  BFRES(std::shared_ptr<File> file, quint32 start_offset = 0);

  /// Reads the %BFRES header from the file, and parses it into a Header.
  ///
  /// @return The success of the reading.
  ResultCode ReadHeader();
  /// Reads the %BFRES group dictionaries from the file, and parses each into a ResourceDictionary.
  ///
  /// @return The success of the reading.
  ResultCode ReadDictionaries();

  /// @brief Represents the %BFRES header.
  ///
  /// @todo These unknowns have been figured out and documented on Tockdom.
  ///
  /// @todo The fields here can be documented.
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

  /// Gets the %BFRES header.
  ///
  /// @return The %BFRES header.
  const Header& GetHeader() const;
  /// Sets the %BFRES header.
  ///
  /// @param  header  The %BFRES header.
  void SetHeader(const Header& header);

  /// The possible endian values of the %BFRES file.
  enum class Endianness
  {
    Little = 0xFFFE,
    Big = 0xFEFF
  };

  /// Gets the endian names.
  ///
  /// @return The endian names mapped to the values.
  static std::map<Endianness, QString> GetEndianNames();
  /// Gets the current endian name.
  ///
  /// @return The current endian name.
  const QString& GetEndianName() const;

  /// Gets the FMDL ResourceDictionary.
  ///
  /// @return A shared pointer to the FMDL ResourceDictionary.
  std::shared_ptr<ResourceDictionary<FMDL>> GetFMDLDictionary() const;
  /// Gets the FTEX ResourceDictionary.
  ///
  /// @return A shared pointer to the FTEX ResourceDictionary.
  std::shared_ptr<ResourceDictionary<FTEX>> GetFTEXDictionary() const;

private:
  /// The size of the main %BFRES header.
  static constexpr quint32 HEADER_SIZE = 0x6C;
  /// The number of groups in a %BFRES.
  static constexpr quint32 NUM_GROUPS = 12;

  /// Reference for what the name of each endian value is.
  inline static const std::map<Endianness, QString> m_endian_names{
      {Endianness::Little, "Little Endian"}, {Endianness::Big, "Big Endian"}};

  /// The %BFRES header parsed from the file.
  Header m_header;

  /// The groups in a BFRES file.
  enum class GroupType
  {
    FMDL = 0,
    FTEX = 1
  };

  /// The current endian name, parsed from the header.
  ///
  /// @todo This is not updated when the frontend sets the header. If the frontend submits a header
  /// with a new BOM, this will be incorrect. Do something like FTEX::SetupComponentNames().
  QString m_endian_name;

  /// Shared pointer to the FMDL group's ResourceDictionary.
  std::shared_ptr<ResourceDictionary<FMDL>> m_fmdl_dictionary;
  /// Shared pointer to the FTEX group's ResourceDictionary.
  std::shared_ptr<ResourceDictionary<FTEX>> m_ftex_dictionary;
};
