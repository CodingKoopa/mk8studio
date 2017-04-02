#ifndef NODE_H
#define NODE_H

#include <QScrollArea>
#include <QStandardItem>

#include "common.h"
#include "filebase.h"

class Node : public QObject
{
  Q_OBJECT
public:
  explicit Node(QObject* parent = 0) : QObject(parent) {}
  // Optional because not every file type may have a file tree to show.
  // TODO: maybe a better way of going about making this optional?
  virtual ResultCode LoadFileTreeArea() { return RESULT_SUCCESS; }
  // Every format must have attributes to show.
  virtual ResultCode LoadAttributeArea() = 0;
  // Same as file trees.
  virtual ResultCode LoadMainWidget() { return RESULT_SUCCESS; }
  // Optional.
  virtual ResultCode SaveFile() { return RESULT_SUCCESS; }
signals:
  void NewFileTreeArea(QScrollArea*);
  void NewAttributesArea(QScrollArea*);
  void NewMainWidget(QWidget*);
  void ConnectNode(Node*);
  void NewStatus(ResultCode status = RESULT_SUCCESS, QString message = QString());

private slots:
  virtual void HandleAttributeItemChange(QStandardItem* item) = 0;
};

#endif  // NODE_H
