#include "BFRES.h"

BFRES::BFRES(File* file) : m_file(file)
{
}

BFRES::BFRES(const BFRES& other)
    : FormatBase(other), m_header(other.m_header),
      m_index_group_headers(other.m_index_group_headers), m_node_blacklist(other.m_node_blacklist),
      m_file(other.m_file)
{
  DeepCopyNodes(other);
}

BFRES& BFRES::operator=(const BFRES& other)
{
  m_header = other.m_header;
  m_index_group_headers = other.m_index_group_headers;
  m_node_blacklist = other.m_node_blacklist;
  m_file = other.m_file;
  DeepCopyNodes(other);
  return *this;
}

BFRES::~BFRES()
{
  foreach (const QVector<Node*>& group, m_raw_node_lists)
    DeleteSubtreeFromNode(group[0]);
}

ResultCode BFRES::ReadHeader()
{
  // TODO: read all the read 32s, read 8s in different for loops like so:
  // https://github.com/citra-emu/citra/blob/master/src/citra_qt/main.cpp#L115
  m_header.magic = m_file->ReadStringASCII(4);
  m_header.unknown_a = m_file->Read8();
  m_header.unknown_b = m_file->Read8();
  m_header.unknown_c = m_file->Read8();
  m_header.unknown_d = m_file->Read8();
  // ACTUAL      WIKI   SKIP()   READ()
  // 00          1      1        read8()
  // 00 00       2      2        read16()
  // 00 00 00 00 4      4        read32()
  m_header.bom = m_file->Read16();
  m_header.unknown_e = m_file->Read16();
  m_header.length = m_file->Read32();
  m_header.alignment = m_file->Read32();
  m_header.file_name_offset = m_file->Read32RelativeOffset();
  m_header.string_table_length = m_file->Read32();
  m_header.string_table_offset = m_file->Read32RelativeOffset();
  for (int i = 0; i < 12; ++i)
    m_header.file_offsets << m_file->Read32RelativeOffset();
  for (int i = 0; i < 12; ++i)
    m_header.file_counts << m_file->Read16();

  m_header.unknown_f = m_file->Read32();

  if (m_file->Pos() != 0x6C)
    return ResultCode::IncorrectHeaderSize;

  m_file->Seek(m_header.file_name_offset);
  // TODO: apparantly the string table stores the lengths of strings too, so a better way of doing
  // this might be seeking back 4 bytes, reading the length, and calling readstring with that length
  m_header.file_name = m_file->ReadStringASCII(0);
  return ResultCode::Success;
}

ResultCode BFRES::ReadIndexGroups()
{
  m_index_group_headers.resize(m_header.file_offsets.size());
  m_raw_node_lists.resize(m_header.file_offsets.size());
  for (int group = 0; group < m_header.file_offsets.size(); ++group)
  {
    if (m_header.file_offsets[group] == 0)
    {
#ifdef DEBUG
      qDebug("Skipping group number %i.", group);
#endif
      m_raw_node_lists[group].append(nullptr);
      continue;
    }
#ifdef DEBUG
    else
      qDebug("Now reading group number %i. This group has %i file(s).", group,
             m_header.file_counts[group]);
#endif
    m_file->Seek(m_header.file_offsets[group]);

    m_index_group_headers[group].length = m_file->Read32();
    m_index_group_headers[group].num_entries = m_file->Read32();

    // +1 because the number of entries excludes the root node
    m_raw_node_lists[group].resize(m_index_group_headers[group].num_entries + 1);

    Node* root_node = ReadNodeAtOffset(m_header.file_offsets[group] + 0x8);
    m_raw_node_lists[group][0] = root_node;
    for (quint32 node = 1; node < m_index_group_headers[group].num_entries + 1; ++node)
      m_raw_node_lists[group][node] =
          ReadNodeAtOffset(m_header.file_offsets[group] + 0x8 + node * 0x10);

    ReadSubtreeFromNode(root_node, group);
    m_node_blacklist.clear();

#ifdef DEBUG
    qDebug("Root node. Search: %08X Byte Index From Left: %08X Byte Index: "
           "%08X Left Index: %i Right Index: %i Name Pointer: %08X Data "
           "Pointer: %08X",
           rootNode.searchVal, rootNode.searchVal >> 3, rootNode.searchVal & 7, rootNode.leftIndex,
           rootNode.rightIndex, rootNode.namePtr, rootNode.dataPtr);
#endif

#ifdef DEBUG
    qDebug("Node %i. Search: %08X Byte Index From Left: %08X Byte Index: "
           "%08X Left Index: %i Right Index: %i Name Pointer: %08X Data "
           "Pointer: %08X",
           b, node.searchVal, node.searchVal >> 3, node.searchVal & 7, node.leftIndex,
           node.rightIndex, node.namePtr, node.dataPtr);
#endif
  }
  return ResultCode::Success;
}

const BFRES::Header& BFRES::GetHeader() const
{
  return m_header;
}

void BFRES::SetHeader(const Header& header)
{
  m_header = header;
}

File* BFRES::GetFile() const
{
  return m_file;
}

const QVector<QVector<BFRES::Node*>>& BFRES::GetRawNodeLists()
{
  return m_raw_node_lists;
}

void BFRES::ReadSubtreeFromNode(Node* node, quint32 group)
{
  if (node)
  {
    // If there is a left index, and it's not referring to the current one, and it's unitialized,
    // and it's not on the blacklist.
    if (node->left_index != 0 && m_raw_node_lists[group].indexOf(node) != node->left_index &&
        !node->left_node && !m_node_blacklist.contains(node->left_index))
    {
      m_node_blacklist.append(node->left_index);

      // Get index of current node
      node->left_node = m_raw_node_lists[group][node->left_index];
      ReadSubtreeFromNode(node->left_node, group);
    }
    else
      node->left_node = nullptr;

    if (node->right_index != 0 && m_raw_node_lists[group].indexOf(node) != node->right_index &&
        !node->right_node && !m_node_blacklist.contains(node->right_index))
    {
      m_node_blacklist.append(node->right_index);

      node->right_node = m_raw_node_lists[group][node->right_index];
      ReadSubtreeFromNode(node->right_node, group);
    }
    else
      node->right_node = nullptr;
  }
}

void BFRES::CopySubtreeFromNode(BFRES::Node* source_node, BFRES::Node* destination_node)
{
  // The source node should have already went through ReadSubtreeFromNode(), so we shouldn't have to
  // do all the checking again.
  if (source_node->left_node)
  {
    destination_node->left_node = new Node;
    CopyNode(source_node->left_node, destination_node->left_node);
  }
  if (source_node->right_node)
  {
    destination_node->right_node = new Node;
    CopyNode(source_node->right_node, destination_node->right_node);
  }
}

void BFRES::DeleteSubtreeFromNode(BFRES::Node* node)
{
  if (node)
  {
    if (node->left_index && node->left_node)
      DeleteSubtreeFromNode(node->left_node);
    if (node->right_index && node->right_node)
      DeleteSubtreeFromNode(node->right_node);
    delete node;
  }
}

BFRES::Node* BFRES::ReadNodeAtOffset(quint64 offset)
{
  m_file->Seek(offset);
  Node* node = new Node;
  node->search_value = m_file->Read32();
  node->left_index = m_file->Read16();
  node->right_index = m_file->Read16();
  node->name_ptr = m_file->Read32RelativeOffset();
  node->data_ptr = m_file->Read32RelativeOffset();
  quint64 pos = m_file->Pos();
  m_file->Seek(node->name_ptr);
  if (node->name_ptr)
    node->name = m_file->ReadStringASCII(0);
  m_file->Seek(pos);
  return node;
}

void BFRES::DeepCopyNodes(const BFRES& other)
{
  m_raw_node_lists.resize(other.m_header.file_offsets.size());

  // Use the other object's lists because this object's are empty.
  for (int group = 0; group < other.m_raw_node_lists.size(); ++group)
  {
    for (quint32 node = 0; node < other.m_index_group_headers[group].num_entries + 1; ++node)
    {
      if (other.m_header.file_offsets[group])
      {
        m_raw_node_lists[group] << new Node;
        CopyNode(other.m_raw_node_lists[group][node], m_raw_node_lists[group][node]);
      }
      else
        m_raw_node_lists[group] << nullptr;

      if (other.m_raw_node_lists[group][0])
        CopySubtreeFromNode(other.m_raw_node_lists[group][0], m_raw_node_lists[group][0]);
    }
  }
}

void BFRES::CopyNode(BFRES::Node* source_node, BFRES::Node* destination_node)
{
  // Manually make a deep copy of the node so the left/right node pointers don't point to the
  // same node.
  destination_node->search_value = source_node->search_value;
  destination_node->left_index = source_node->left_index;
  destination_node->right_index = source_node->right_index;
  destination_node->name_ptr = source_node->name_ptr;
  destination_node->data_ptr = source_node->data_ptr;
  destination_node->name = source_node->name;
}
