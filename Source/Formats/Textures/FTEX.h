#pragma once

#include <QString>

#include "File.h"
#include "Formats/Textures/GX2ImageBase.h"

class FTEX : public GX2ImageBase
{
public:
  FTEX() = default;
  FTEX(File* file, quint64 start_offset);
  FTEX(const FTEX& other);
  FTEX& operator=(const FTEX& other);
  ~FTEX();

  ResultCode ReadHeader();
  ResultCode ReadImage();

  void InjectImage();

  const QVector<QString> GetComponentNameList() const;
  quint8 GetComponentIDFromName(const QString& name) const;

  quint64 GetStart() const;

  struct Header : ImageHeaderBase
  {
    QString magic;
    quint32 dimension;
    quint32 usage;
    quint32 data_offset_runtime;
    quint32 mipmap_offset_runtime;
    quint32 alignment;
    QVector<quint32> mipmap_offsets;
    quint32 first_mipmap;
    quint32 num_mips_alt;
    quint32 first_slice;
    quint32 num_slices;
    quint8 red_channel_component;
    quint8 green_channel_component;
    quint8 blue_channel_component;
    quint8 alpha_channel_component;
    QVector<quint32> registers;
    quint32 texture_handle_runtime;
    quint32 array_length;
    qint32 file_name_offset;
    qint32 file_path_offset;
    qint32 data_offset;
    qint32 mipmap_section_offset;
    qint32 user_data_index_group_offset;
    quint16 user_data_entry_count;
  };

  const Header& GetHeader() const;
  void SetHeader(const Header& ftex_header);

  struct Component
  {
    quint8 id;
    QString name;
  };

private:
  void DeepCopyRawImageDataBuffer(const FTEX& other);

  static constexpr quint32 HEADER_SIZE = 0xC0;

  const inline static QVector<QString> m_component_name_list{"Texture Red Values",
                                                             "Texture Green Values",
                                                             "Texture Blue Values",
                                                             "Texture Alpha Values",
                                                             "Always 0",
                                                             "Always 1"};

  Header m_header = Header();
  char* m_raw_image_data_buffer = nullptr;
};
