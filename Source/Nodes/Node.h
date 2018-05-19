#pragma once

#include <QScrollArea>
#include <QStandardItem>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "Common.h"
#include "File.h"
#include "QtUtils/DynamicItemDelegate.h"
#include "QtUtils/DynamicStandardItem.h"

class DynamicStandardItem;
class Node : public QObject
{
  Q_OBJECT
public:
  explicit Node(QObject* parent = 0);
  // Optional because not every file type may have a file tree to show.
  // TODO: maybe a better way of going about making this optional?
  virtual ResultCode LoadFileTreeArea() { return ResultCode::NotAvailable; }
  virtual DynamicStandardItem* MakeItem() = 0;
  // Every format must have attributes to show.
  virtual ResultCode LoadAttributeArea() = 0;
  // Same as file trees.
  virtual ResultCode LoadMainWidget() { return ResultCode::NotAvailable; }
  // Optional.
  virtual ResultCode SaveFile() { return ResultCode::NotAvailable; }

protected:
  // List of what each table cell's editor should be.
  DynamicItemDelegate::DelegateInfo m_delegate_group;
  QTreeView* m_tree_view;
  QWidget* m_main_widget;
  QMenu* m_context_menu;

  // Helper function for MakeItem(). TODO: Better name for this.
  DynamicStandardItem* MakeLabelItem(QString label);
  QScrollArea* MakeAttributeSection(QStandardItemModel* table_view_layout);

signals:
  void ConnectNode(Node*);
  void NewFileTreeArea(QScrollArea*);
  void NewAttributeArea(QScrollArea*);
  void NewMainWidget(QWidget*);
  void NewStatus(ResultCode status = ResultCode::Success, QString message = QString());

protected slots:
  void HandleFileTreeClick(QModelIndex index);
  void HandleTreeCustomContextMenuRequest(const QPoint& point);
  virtual void HandleAttributeItemChange(QStandardItem*) { return; }
};
