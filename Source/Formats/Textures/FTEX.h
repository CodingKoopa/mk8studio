#pragma once

#include <array>
#include <memory>

#include <QString>

#include "File.h"
#include "Formats/Textures/GX2ImageBase.h"

/// @brief Represents a ca<b>f</b>e <b>tex</b>ture (%FTEX).
class FTEX : public GX2ImageBase
{
public:
  /// Initializes a new instance of the FTEX class.
  ///
  /// @param  file          Shared pointer to the file to read from.
  /// @param  start_offset  The offset to start reading at, where the BFRES structure starts.
  FTEX(std::shared_ptr<File> file, quint64 start_offset = 0);
  /// Initializes a new instance of the FTEX class from an existing copy. Deleted so that the image
  /// data buffer is not double deleted.
  FTEX(const FTEX&) = delete;
  /// Assigns this instance of the FTEX class to a new one. Deleted so that the image data buffer
  /// is not double deleted.
  FTEX& operator=(const FTEX&) = delete;
  /// Uninitializes an instance of the DDS class, deleting the image data buffer.
  ~FTEX();

  /// Reads the %FTEX header from the file, and parses it into a Header.
  ///
  /// @return The success of the reading.
  ResultCode ReadHeader();
  /// Reads the raw image from the file, and deswizzles it with GX2ImageBase::ReadImageFromData().
  ///
  /// @return The success of the reading.
  ResultCode ReadImage();

  /// Writes the %FTEX header and swizzled image data to the file. Temporary, until %BFRES
  /// rebuilding becomes possible for this tool.
  void InjectImage();

  /// Gets the component name reference.
  ///
  /// @return The component name reference.
  static std::array<QString, 6> GetComponentNames();

  /// @brief Represents the FTEX header.
  ///
  /// @todo The fields here can be documented.
  struct Header : HeaderBase
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

  /// Gets the %FTEX header.
  ///
  /// @return The %FTEX header.
  const Header& GetHeader() const;
  /// Sets the %FTEX header.
  ///
  /// @param  header  The %FTEX header.
  void SetHeader(const Header& header);

private:
  /// The size of the main %FTEX header.
  static constexpr quint32 HEADER_SIZE = 0xC0;

  /// Reference for what the name of each component is. Unlike the other references, the entries
  /// here are consecutive, the ID for one entry comes right after the previous one. Therefore, we
  /// can use an array instead of a key/value model.
  inline static const std::array<QString, 6> m_component_names{"Texture Red Values",
                                                               "Texture Green Values",
                                                               "Texture Blue Values",
                                                               "Texture Alpha Values",
                                                               "Always 0",
                                                               "Always 1"};

  /// The %FTEX header parsed from the file.
  Header m_header = Header();

  /// The image data buffer, temporarily used when moving image data to and from a %FTEX file.
  char* m_raw_image_data_buffer = nullptr;
};
