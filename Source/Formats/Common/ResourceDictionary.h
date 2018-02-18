#pragma once

#include "Common.h"
#include "File.h"
#include "Formats/FormatBase.h"

template <typename T>
class ResourceDictionary : public FormatBase
{
public:
  // This must be defined here and not in a CPP file, because of how templates work.
  ResourceDictionary(File* file = nullptr, quint32 start_offset = 0)
      : FormatBase(file, start_offset)
  {
  }

  ResourceDictionary(const ResourceDictionary& other)
      : FormatBase(other.m_file, other.m_start_offset), m_header(other.m_header),
        m_node_list(other.m_node_list)
  {
  }

  ResourceDictionary& operator=(const ResourceDictionary& other)
  {
    m_file = other.m_file;
    m_start_offset = other.m_start_offset;
    m_header = other.m_header;
    m_node_list = other.m_node_list;
    return *this;
  }

  struct Header
  {
    quint32 size;
    qint32 num_nodes;
  };

  struct Node
  {
    quint32 search_value;
    quint16 left_index;
    quint16 right_index;
    qint32 key_offset;
    qint32 value_offset;

    QString key;
    T value;
  };

  // Expose read-only references to nodes via [].
  Node operator[](quint32 index) const { return m_node_list[index]; }
  // Expose modifiable references to nodes via [].
  Node& operator[](quint32 index) { return m_node_list[index]; }

  quint32 Size() const { return m_node_list.size(); }

  ResultCode ReadHeader()
  {
    m_file->Seek(m_start_offset);
    m_header.size = m_file->ReadU32();
    m_header.num_nodes = m_file->ReadS32();
    // TODO: header check
    return ResultCode::Success;
  }

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
      m_node_list[node].value = T(m_file, m_node_list[node].value_offset);
      m_node_list[node].value.SetName(m_node_list[node].key);
    }

    // TODO: Utility function for checking header size.
    return ResultCode::Success;
  }

  const Header& GetHeader() const { return m_header; }

  const QVector<Node>& GetNodeList() const { return m_node_list; }

private:
  static constexpr quint32 HEADER_SIZE = 0x08;
  static constexpr quint32 NODE_SIZE = 0x10;

  Header m_header;
  QVector<Node> m_node_list;
};
