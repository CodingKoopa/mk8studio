#pragma once

#include <QVector>

#include "Common.h"
#include "File.h"
#include "FormatBase.h"

class BFRES : public FormatBase
{
public:
  BFRES(File* file);
  // Copy Constructor
  BFRES(const BFRES& other);
  BFRES& operator=(const BFRES& other);
  ~BFRES();

  struct Header
  {
    QString magic;
    quint8 unknown_a;
    quint8 unknown_b;
    quint8 unknown_c;
    quint8 unknown_d;
    quint16 bom;
    quint16 unknown_e;
    quint32 length;
    quint32 alignment;
    quint32 file_name_offset;
    quint32 string_table_length;
    quint32 string_table_offset;
    QVector<quint32> file_offsets;
    QVector<quint16> file_counts;
    quint32 unknown_f;

    QString file_name;
  };

  struct IndexGroupHeader
  {
    quint32 length;
    quint32 num_entries;
  };

  struct Node
  {
    quint32 search_value;
    quint16 left_index;
    quint16 right_index;
    quint32 name_ptr;
    quint32 data_ptr;

    QString name;
    Node* left_node = nullptr;
    Node* right_node = nullptr;
  };

  enum class GroupType
  {
    FMDL = 0,
    FTEX = 1
  };

  ResultCode ReadHeader();
  ResultCode ReadIndexGroups();

  const Header& GetHeader() const;
  void SetHeader(const Header& header);

  File* GetFile() const;

  const QVector<QVector<Node*>>& GetRawNodeLists();

private:
  void ReadSubtreeFromNode(Node* node, quint32 group);
  void CopySubtreeFromNode(Node* source_node, Node* destination_node);
  void DeleteSubtreeFromNode(Node* node);

  Node* ReadNodeAtOffset(quint64 offset);
  void DeepCopyNodes(const BFRES& other);
  void CopyNode(Node* source_node, Node* destination_node);

  // http://mk8.tockdom.com/wiki/BFRES_(File_Format)
  Header m_header;
  QVector<IndexGroupHeader> m_index_group_headers;

  QVector<QVector<Node*>> m_raw_node_lists;
  QVector<int> m_node_blacklist;

  File* m_file;
};
