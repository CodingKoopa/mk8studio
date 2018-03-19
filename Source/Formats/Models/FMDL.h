#pragma once

#include <memory>

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"
#include "Formats/Models/FVTX.h"

/// Represents a ca<b>f</b>e <b>m</b>o<b>d</b>e<b>l</b> (%FMDL).
///
/// %FMDL structures contain the composition of the model, defining the verticies, how they are put
/// together, and what attributes they have. Although what texture is assigned to surfaces is
/// specified here, the textures themselves are stored in FTEXs.
class FMDL : public FormatBase
{
public:
  /// Initializes a new instance of the FMDL class.
  ///
  /// @param    file            Shared pointer to the file to read from.
  /// @param    start_offset    The offset to start reading at, where the BFRES structure starts.
  FMDL(std::shared_ptr<File> file, quint32 start_offset = 0);

  /// Reads the %FMDL header from the file, and parses it into a Header.
  ///
  /// @return   The success of the reading.
  ResultCode ReadHeader();
  /// Reads the array of FVTXs from the file, and parses each into an FVTX of the list.
  ///
  /// @return   The success of the reading.
  ResultCode ReadFVTXArray();

  /// Represents the %FMDL header.
  ///
  /// @todo The fields here can be documented.
  struct Header
  {
    QString magic;
    qint32 file_name_offset;
    qint32 file_path_offset;
    qint32 fskl_offset;
    qint32 fvtx_array_offset;
    qint32 fshp_index_group_offset;
    qint32 fmat_index_group_offset;
    qint32 user_data_index_group_offset;
    quint16 fvtx_count;
    quint16 fshp_count;
    quint16 fmat_count;
    quint16 user_data_entry_count;
    quint32 num_vertices;
    quint32 user_pointer_runtime;
  };

  /// Gets the %FMDL header.
  ///
  /// @return   The %FMDL header.
  const Header& GetHeader() const;
  /// Sets the %FMDL header.
  ///
  /// @param    header  The %FMDL header.
  void SetHeader(const Header& header);

  /// Convenience type for a list of FVTX objects.
  ///
  /// For more info on why this kind of complex typing with smart pointers is necessary, see
  /// @ref Communication "this".
  typedef std::shared_ptr<std::vector<std::shared_ptr<FVTX>>> FVTXList;

  /// Gets the list of FVTX objects representing the %FVTX array.
  ///
  /// @return   The FVTX list.
  FVTXList GetFVTXList() const;

private:
  /// The size of the main %FMDL header.
  static constexpr quint32 HEADER_SIZE = 0x30;

  /// The %FMDL header parsed from the file.
  Header m_header;

  /// The list of FVTX objects parsed from the %FVTX array.
  FVTXList m_fvtx_list;
};
