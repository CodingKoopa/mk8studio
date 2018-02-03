#pragma once

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"

class FVTX : public FormatBase
{
public:
  FVTX(File* file = nullptr, quint32 start_offset = 0);
  ResultCode ReadHeader();
  ResultCode ReadAttributes();

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

  struct Attribute
  {
    qint32 name_offset;
    quint8 buffer_index;
    quint16 buffer_offset;
    quint32 format;
    QString name;
  };

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

  const Header& GetHeader() const;
  void SetHeader(const Header& header);

  const QVector<FVTX::Attribute>& GetAttributeList() const;

  const QVector<AttributeNameInfo>& GetAttributeNameInfoList() const;

private:
  // TODO: add the rest.
  // clang-format off
  const std::map<quint32, QString> m_attribute_format_names{
      // Unsigned Normals
      {0x0,     "GX2_ATTRIB_FORMAT_UNORM_8"},
      {0x4,     "GX2_ATTRIB_FORMAT_UNORM_8_8"},
      {0x7,     "GX2_ATTRIB_FORMAT_UNORM_16_16"},
      {0xA,     "GX2_ATTRIB_FORMAT_UNORM_8_8_8_8"},
      // Unsigned Integer
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
  QVector<QString> m_attribute_format_name_list;

  // clang-format off
  const std::map<QString, AttributeNameInfo> m_attribute_name_infos{
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
  QVector<AttributeNameInfo> m_attribute_name_info_list;

  Header m_header;

  QVector<Attribute> m_attribute_list;
};
