#pragma once

#include <functional>

#include <QScrollArea>
#include <QStandardItem>
#include <QTableView>
#include <QTableWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>

#include "Common.h"
#include "File.h"
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

  typedef std::function<void(QVariant)> HandleEditFunction;
  typedef QPair<QVector<QString>, quint32> AttributeComboBoxData;

  /// Contains info about a row of an attribute table.
  struct AttributeTableRow
  {
    /// The name of the attribute. This will be used in the first column.
    QString attribute_name;
    /// The value of the attribute. This will be used in the second column.
    QVariant attribute_value;
    // Whether the value of the attribute should be editable or not. This will be used to set the
    // flags of the second column.
    bool editable = false;
    // The function to execute if the value is edited.
    HandleEditFunction handle_edit{};
  };

protected:
  QTreeView* m_tree_view;
  QWidget* m_main_widget;
  QMenu* m_context_menu;

  // Helper function for MakeItem(). TODO: Better name for this.
  DynamicStandardItem* MakeLabelItem(QString label);
  /// @todo This should be made a static function, but "this" is required to handle
  /// QTableWidget::itemChanged.
  QScrollArea* MakeAttributeSection(const QVector<AttributeTableRow>& table_rows,
                                    std::function<void()> update_data = {});
  QScrollArea* MakeAttributeSectionOld(QStandardItemModel* table_view_layout);

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

// Enable functions as a metatype, to store in an item.
Q_DECLARE_METATYPE(Node::HandleEditFunction)
Q_DECLARE_METATYPE(Node::AttributeComboBoxData)