#pragma once

#include <memory>

#include <QString>

#include "Common.h"
#include "File.h"

/// @brief Abstracts common aspects of file formats.
class FormatBase
{
public:
  /// Initializes a new instance of the FMDL class. There must be an constructor usable without
  /// parameters, for stuff like the DDS class that may not have a File right away.
  ///
  /// @param  file          Shared pointer to the file to read from.
  /// @param  start_offset  The offset to start reading at, where the FVTX structure starts.
  /// @param  header_size   The size of the format header.
  FormatBase(std::shared_ptr<File> file = nullptr, quint32 start_offset = 0,
             quint32 header_size = 0);

  /// Gets the file path.
  ///
  /// @return The file path.
  const QString& GetPath() const;
  /// Sets the file path.
  ///
  /// @param  path  The file path.
  void SetPath(const QString& path);

  /// Gets the internal name.
  ///
  /// @return The internal name.
  const QString& GetName() const;
  /// Sets the internal name.
  ///
  /// @param  name  The internal name.
  void SetName(const QString& name);

  /// Gets the header size.
  ///
  /// @return The header size.
  quint32 GetHeaderSize() const;

protected:
  /// Verifies that the correct amount of bytes was read, for a given structure.
  ///
  /// @param  start_pos   The offset that reading started at. This is mandatory because, if made
  /// an optional argument, whether the argument is specified or not is checked by comparing the
  /// offset to nullptr. 0, a completely valid start offset, looks identical to a nullptr.
  /// @param  header_size The size of the structure read. Since 0 is not a valid structure size,
  /// we can rule out it as a possible parameter value, and be a default value. If this is not
  /// specified, it will default to the member header size.
  ///
  /// @return Result code, with ResultCode::Success being returned when the verification succeeded,
  /// and ResultCode::IncorrectHeaderSize being returned when anything fails or doesn't check out.
  ///
  /// @todo The start_pos variable name is inconsistent. In other places, "start_offset" is used,
  /// and should be preferred here.
  ///
  /// @todo The name of this method is misleading. It can be used not only for headers, but other
  /// kinds of structures too.
  ResultCode CheckHeaderSize(quint32 start_pos, quint32 header_size = 0);

  /// The main file to read from. For traditional Wii U formats, this should always be valid. For
  /// other formats like DDS, it may go unused in favor of reading and writing to separate files,
  /// with File objects being initialized on the spot.
  std::shared_ptr<File> m_file;
  /// The offset to start reading at.
  quint32 m_start_offset;
  /// The size of the format header.
  quint32 m_header_size;

  /// The path to the file. This is not a strict field, it may or may not be set.
  QString m_path;
  /// The internal name. This is also not a strict field, it may or may not be set, and can be used
  /// in different ways.
  QString m_name;
};
