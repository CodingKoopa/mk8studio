#pragma once

#include <memory>

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"

/// @brief Represents a ca<b>f</b>e <b>v</b>er<b>t</b>e<b>x</b> (%FVTX).
///
/// %FVTX structures are groups of verticies making up a model. This directs the reader towards the
/// raw vertex data, and the respective vertex attributes to go along with them.
class FVTX : public FormatBase
{
public:
  /// Initializes a new instance of the FMDL class.
  ///
  /// @param  file            Shared pointer to the file to read from.
  /// @param  start_offset    The offset to start reading at, where the FVTX structure starts.
  FVTX(std::shared_ptr<File> file, quint32 start_offset = 0);

  /// Reads the %FVTX header from the file, and parses it into a Header.
  ///
  /// @return The success of the reading.
  ResultCode ReadHeader();
  /// Reads the array of %FVTX vertex attributes from the file, and parses each into an Attribute of
  /// the list.
  ///
  /// @return The success of the reading.
  ResultCode ReadAttributes();
  /// Reads the %FVTX vertex buffers from the file, and parses each into a Buffer.
  ///
  /// @return The success of the reading.
  ResultCode ReadBuffers();

  /// @brief Represents the %FVTX header.
  ///
  /// @todo The fields here can be documented.
  struct Header
  {
    QString magic;
    quint8 attribute_count;
    quint8 buffer_count;
    quint16 section_index;
    quint32 number_vertices;
    quint8 vertex_skin_count;
    qint32 attribute_array_offset;
    qint32 attribute_index_group_offset;
    qint32 buffer_array_offset;
    quint32 user_pointer_runtime;
  };

  /// Gets the %FVTX header.
  ///
  /// @return The %FVTX header.
  const Header& GetHeader() const;
  /// Sets the %FVTX header.
  ///
  /// @param  header  The %FVTX header.
  void SetHeader(const Header& header);

  /// @brief Represents an attribute.
  ///
  /// For this particular %FVTX, this will, for a given attribute (e.g. positioning, U.V. mapping),
  /// define the format to be used. Additionally, it points to where the buffer containing the data
  /// for every vertex is in the %BFRES file. This is different from the @ref qtfrontendattributes.
  ///
  /// @todo The fields here can be documented.
  struct Attribute
  {
    qint32 name_offset;
    quint8 buffer_index;
    quint16 buffer_offset;
    quint32 format;
    QString name;
  };

  /// Gets the list of Attribute objects representing the attribute array.
  ///
  /// @return The Attribute list.
  const QVector<Attribute>& GetAttributeList() const;

  /// Gets the attribute format name info reference.
  ///
  /// @return The Attribute format name info reference.
  static std::map<quint32, QString> GetAttributeFormatNames();
  /// Gets the list of parsed attribute format names.
  ///
  /// @return The Attribute format names.
  const QVector<QString>& GetAttributeFormatNameList() const;

  /// @brief Contains info about an attribute that can be collected from its name.
  ///
  /// @todo The fields here can be documented.
  struct AttributeNameInfo
  {
    enum class Use
    {
      Position,
      Normal,
      Tangent,
      Binormal,
      BlendWeight,
      BlendIndex,
      UV0,
      UV1,
      UV2,
      UV3,
      Color0,
      Color1,
    } use;
    QString friendly_name;
  };

  /// Gets the list of attribute name info objects.
  ///
  /// @return The AttributeNameInfo list.
  const QVector<AttributeNameInfo>& GetAttributeNameInfoList() const;

  /// @brief Represents a buffer.
  ///
  /// Buffers are referred to by vertex Attribute structures. Where
  /// Attribute structures describe meta info about an attribute, Buffer structures contain the
  /// actual data for the attribute.
  ///
  /// @todo The fields here can be documented.
  struct Buffer
  {
    quint32 data_pointer_runtime;
    quint32 size;
    quint32 buffer_handle_runtime;
    quint16 stride;
    quint16 buffering_count;
    quint32 context_pointer_runtime;
    qint32 data_offset;
  };

  /// Gets the list of Buffer objects representing the buffer array.
  ///
  /// @return The Buffer list.
  const QVector<Buffer>& GetBufferList() const;

private:
  /// The size of the main %FMDL header.
  static constexpr quint32 HEADER_SIZE = 0x20;
  /// The size of an attribute.
  static constexpr quint32 ATTRIBUTE_SIZE = 0x0C;
  /// The size of a buffer.
  static constexpr quint32 BUFFER_SIZE = 0x18;

  /// Reference for the name of each format, mapping ech format ID with Decaf's name for it. As
  /// buffer reading is implemented, this will probably have to be expanded to map the IDs to an
  /// AttributeFormatInfo struct.
  ///
  /// @todo Add the rest of the known formats.
  // clang-format off
  inline static const std::map<quint32, QString> m_attribute_format_names{
      // Unsigned Normals
      {0x0,     "GX2_ATTRIB_FORMAT_UNORM_8"},
      {0x4,     "GX2_ATTRIB_FORMAT_UNORM_8_8"},
      {0x7,     "GX2_ATTRIB_FORMAT_UNORM_16_16"},
      {0xA,     "GX2_ATTRIB_FORMAT_UNORM_8_8_8_8"},
      // Unsigned Integer
      {0x0100,  "GX2_ATTRIB_FORMAT_UINT_8"},
      {0x0104,  "GX2_ATTRIB_FORMAT_UINT_8_8" },
      {0x010A,  "GX2_ATTRIB_FORMAT_UINT_8_8_8_8"},
      // Signed Normals
      {0x0207,  "GX2_ATTRIB_FORMAT_SNORM_16_16"},
      {0x020A,  "GX2_ATTRIB_FORMAT_SNORM_8_8_8_8" },
      {0x020B,  "GX2_ATTRIB_FORMAT_SNORM_10_10_10_2"},
      // Floats
      {0x080D,  "GX2_ATTRIB_FORMAT_FLOAT_32_32"},
      {0x0811,  "GX2_ATTRIB_FORMAT_FLOAT_32_32_32"}};
  // clang-format on

  /// Reference for the info about each attribute name.
  // clang-format off
  inline static const std::map<QString, AttributeNameInfo> m_attribute_name_infos{
      {"_p0", {AttributeNameInfo::Use::Position,    "Position"}},
      {"_n0", {AttributeNameInfo::Use::Normal,      "Normal"}},
      {"_t0", {AttributeNameInfo::Use::Tangent,     "Tangent"}},
      {"_b0", {AttributeNameInfo::Use::Binormal,    "Binormial"}},
      {"_w0", {AttributeNameInfo::Use::BlendWeight, "Blend Weight"}},
      {"_i0", {AttributeNameInfo::Use::BlendIndex,  "Blend Index"}},
      {"_u0", {AttributeNameInfo::Use::UV0,         "UV 0"}},
      {"_u1", {AttributeNameInfo::Use::UV1,         "UV 1"}},
      {"_u2", {AttributeNameInfo::Use::UV2,         "UV 2"}},
      {"_u3", {AttributeNameInfo::Use::UV3,         "UV 3"}},
      {"_c0", {AttributeNameInfo::Use::Color0,      "Color 0"}},
      {"_c1", {AttributeNameInfo::Use::Color1,      "Color 1"}},
  };
  // clang-format on

  /// The %FVTX header parsed from the file.
  Header m_header;

  /// The list of attributes parsed from the file.
  QVector<Attribute> m_attribute_list;

  /// The list of attribute format names parsed from the file.
  ///
  /// The "list" terminology here is chosen for a reason. The "names" suffixes indicate that the
  /// member variable contains the constant references, but this is a list of the real, computed
  /// info parsed from the attribute.
  QVector<QString> m_attribute_format_name_list;

  /// The list of attribute name infos parsed from the file.
  QVector<AttributeNameInfo> m_attribute_name_info_list;

  /// The list of buffers parsed from the file.
  QVector<Buffer> m_buffer_list;
};
