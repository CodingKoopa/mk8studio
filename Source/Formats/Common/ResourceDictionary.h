#pragma once

#include <memory>

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"

/// @brief Represents a resource dictionary, used throughout the %BFRES format.
///
/// Resource dictionaries are radix trees composed of patricia tries that allow for quick lookups
/// of elements. They are also sometimes less accurately referred to as index groups. Despite them
/// being radix trees, here nodes are read in a straightforward manner, ignoring the heirarchy of
/// nodes.
///
/// @tparam T   The type of the value/resource that the dictionary is storing.
template <typename T>
class ResourceDictionary : public FormatBase
{
public:
  /// Initializes a new instance of the ResourceDictionary class.
  ///
  /// @param    file            Shared pointer to the file to read from.
  /// @param    start_offset    The offset to start reading at, where the dictionary starts.
  ResourceDictionary(std::shared_ptr<File> file = nullptr, quint32 start_offset = 0)
      : FormatBase(file, start_offset)
  {
  }

  /// @brief Represents a node within a dictionary, containing a resource.
  ///
  /// This is different from the node classes used in the GUI! These refer to the node data
  /// structures in %BFRES files, GUI nodes are a different concept.
  ///
  /// @todo The fields here can be documented.
  struct Node
  {
    quint32 search_value;
    quint16 left_index;
    quint16 right_index;
    qint32 key_offset;
    qint32 value_offset;

    QString key;
    std::shared_ptr<T> value;
  };

  /// Gets read-only references to nodes by index via []. This follows the ordering in which the
  /// nodes are defined in the file, and not the heirarchy of nodes.
  ///
  /// @param    index   The index of the node.
  ///
  /// @return   A copy of the node at the index.
  Node operator[](quint32 index) const { return m_node_list[index]; }
  /// Gets modifiable references to nodes by index via []. This follows the same ordering as
  /// the other access operator.
  ///
  /// @param    index   The index of the node.
  ///
  /// @return   A reference to the node at the index.
  ///
  /// @todo Properly link to the other access operator.
  Node& operator[](quint32 index) { return m_node_list[index]; }

  /// Reads the resource dictionary header from the file, and parses it into a Header.
  ///
  /// @return   The success of the reading.
  ///
  /// @todo     Verify the number of bytes read.
  ResultCode ReadHeader()
  {
    m_file->Seek(m_start_offset);
    m_header.size = m_file->ReadU32();
    m_header.num_nodes = m_file->ReadS32();
    return ResultCode::Success;
  }

  /// Reads the dictionary nodes from the file, and parses each into a Node.
  ///
  /// @return   The success of the reading.
  ///
  /// @todo     Verify the number of bytes read.
  ResultCode ReadNodes()
  {
    m_node_list.resize(m_header.num_nodes + 1);
    for (qint32 node = 0; node < m_header.num_nodes + 1; ++node)
    {
      m_file->Seek(m_start_offset + HEADER_SIZE + (NODE_SIZE * node));

      // The nodes form a Radix Tree, but it's 100 times easier to read them as a normal list now,
      // and fix it up later when saving.
      m_node_list[node].search_value = m_file->ReadU32();
      m_node_list[node].left_index = m_file->ReadU16();
      m_node_list[node].right_index = m_file->ReadU16();
      m_node_list[node].key_offset = m_file->ReadS32RelativeOffset();
      m_node_list[node].value_offset = m_file->ReadS32RelativeOffset();

      // Temporary, until string table reading.
      m_file->Seek(m_node_list[node].key_offset);
      if (m_node_list[node].key_offset)
        m_node_list[node].key = m_file->ReadStringASCII();
      m_node_list[node].value = std::make_shared<T>(m_file, m_node_list[node].value_offset);
      m_node_list[node].value->SetName(m_node_list[node].key);
    }

    return ResultCode::Success;
  }

  /// Gets the size of the dictionary.
  ///
  /// @return   The number of nodes, not including the root node.
  quint32 Size() const { return m_node_list.size(); }

  /// @brief Represents the Resource Dictionary header.
  ///
  /// @todo The fields here can be documented.
  struct Header
  {
    quint32 size;
    qint32 num_nodes;
  };

  /// Gets the Header. There is no accompanying setter because the Header's fields should not be
  /// modified manually manually externally, but from methods here that can add notes with the whole
  /// dictionary in mind.
  ///
  /// @return   A read-only reference to the Header.
  const Header& GetHeader() const { return m_header; }

  /// Gets the list of nodes.
  ///
  /// @return   A read-only reference to the list of nodes.
  const QVector<Node>& GetNodeList() const { return m_node_list; }

private:
  /// The size of the main resource dictionary header.
  static constexpr quint32 HEADER_SIZE = 0x08;
  /// The size of one node in a resource dictionary.
  static constexpr quint32 NODE_SIZE = 0x10;

  /// The resource dictionary header parsed from the file.
  Header m_header;

  /// The list of nodes in the dictionary. This is a linear list that does not follow the hierarchy.
  QVector<Node> m_node_list;
};
