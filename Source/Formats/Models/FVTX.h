#pragma once

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"

class FVTX : public FormatBase
{
public:
  FVTX() = default;
  FVTX(File* file, quint32 start_offset);
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

  // TODO: see bfres.h
  struct AttributeNameInfo
  {
    enum class Use
    {
      Unknown,
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
    QString internal_name;
    QString friendly_name;
  };

  struct Attribute
  {
    qint32 name_offset;
    quint8 buffer_index;
    quint16 buffer_offset;
    quint32 format;

    QString name;
    AttributeNameInfo name_info;
  };

  const Header& GetHeader() const;
  void SetHeader(const Header& header);

  const QVector<FVTX::Attribute>& GetAttributeList();

private:
  // TODO: This should be const, but making it so deletes this class' default assignment operator,
  // and a class for these info structs will be made in the future.
  QVector<AttributeNameInfo> m_attribute_name_info_list{
      {AttributeNameInfo::Use::Unknown, QString(), "Unknown"},
      {AttributeNameInfo::Use::Position, "_p0", "Position"},
      {AttributeNameInfo::Use::Normal, "_n0", "Normal"},
      {AttributeNameInfo::Use::Tangent, "_t0", "Tangent"},
      {AttributeNameInfo::Use::Binormal, "_b0", "Binormial"},
      {AttributeNameInfo::Use::BlendWeight, "_w0", "Blend Weight"},
      {AttributeNameInfo::Use::BlendIndex, "_i0", "Blend Index"},
      {AttributeNameInfo::Use::UV0, "_u0", "UV 0"},
      {AttributeNameInfo::Use::UV1, "_u1", "UV 1"},
      {AttributeNameInfo::Use::UV2, "_u2", "UV 2"},
      {AttributeNameInfo::Use::UV3, "_u3", "UV 3"},
      {AttributeNameInfo::Use::Color0, "_c0", "Color 0"},
      {AttributeNameInfo::Use::Color1, "_c1", "Color 1"},
  };

  File* m_file = nullptr;
  quint32 m_start_offset = 0;
  Header m_header = Header();

  QVector<Attribute> m_attribute_list = QVector<Attribute>();
};
