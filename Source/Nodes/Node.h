#ifndef NODE_H
#define NODE_H

#include <QScrollArea>
#include <QStandardItem>
#include <QTreeView>

#include "Common.h"
#include "CustomDelegate.h"
#include "FileBase.h"

class Node : public QObject
{
  Q_OBJECT
public:
  explicit Node(QObject* parent = 0)
      : QObject(parent), m_tree_view(nullptr), m_main_widget(nullptr), m_context_menu(nullptr)
  {
  }
  // Optional because not every file type may have a file tree to show.
  // TODO: maybe a better way of going about making this optional?
  virtual ResultCode LoadFileTreeArea() { return ResultCode::NotAvailable; }
  virtual QStandardItem* MakeItem() = 0;
  // Every format must have attributes to show.
  virtual ResultCode LoadAttributeArea() = 0;
  // Same as file trees.
  virtual ResultCode LoadMainWidget() { return ResultCode::NotAvailable; }
  // Optional.
  virtual ResultCode SaveFile() { return ResultCode::NotAvailable; }

  QMenu* GetContextMenu();

protected:
  // List of what each table cell's editor should be.
  CustomItemDelegate::DelegateGroup m_delegate_group;
  QTreeView* m_tree_view;
  QWidget* m_main_widget;
  QMenu* m_context_menu;

signals:
  void NewFileTreeArea(QScrollArea*);
  void NewAttributesArea(QScrollArea*);
  void NewMainWidget(QWidget*);
  void ConnectNode(Node*);
  void NewStatus(ResultCode status = ResultCode::Success, QString message = QString());

protected slots:
  void HandleFileTreeClick(QModelIndex index);
  void HandleTreeCustomContextMenuRequest(const QPoint& point);
  virtual void HandleAttributeItemChange(QStandardItem* item) = 0;
};

#endif  // NODE_H
