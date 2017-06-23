#ifndef BFRES_H
#define BFRES_H

#include <QVector>

#include "Common.h"
#include "FileBase.h"
#include "FormatBase.h"

class BFRES : public FormatBase
{
public:
  BFRES(FileBase* m_file);
  ~BFRES();

  struct BFRESHeader
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

  enum GroupType
  {
    GROUP_FMDL = 0,
    GROUP_FTEX = 1
  };

  ResultCode ReadHeader();
  int ReadIndexGroups();

  BFRESHeader GetHeader();
  void setHeader(BFRESHeader value);

  // TODO: Reorder to put all get/set functions in the same place.
  FileBase* GetFile();

  QVector<Node*> GetRootNodes();
  void SetRootNodes(QVector<Node*> root_nodes);

  QVector<QVector<Node*>> GetRawNodeLists();
  void SetRawNodeLists(QVector<QVector<Node*>> raw_node_lists);

private:
  void ReadSubtreeFromNode(Node* node, quint32 group);
  void DeleteSubtreeFromNode(Node* node);

  Node* ReadNodeAtOffset(quint64 offset);

  // http://mk8.tockdom.com/wiki/BFRES_(File_Format)
  BFRESHeader m_header;
  QVector<IndexGroupHeader> m_index_group_headers;

  QVector<Node*> m_root_nodes;

  QVector<QVector<Node*>> m_raw_node_lists;
  QVector<int> m_node_blacklist;

  FileBase* m_file;
};

#endif  // BFRES_H
