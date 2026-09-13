/**
 * @file DateTableWidgetItem.h
 * @brief Table widget item that sorts rows by an embedded date value.
 */

#ifndef JS8CALL_DATETABLEWIDGETITEM_H
#define JS8CALL_DATETABLEWIDGETITEM_H

#include <QTableWidgetItem>

/**
 * @class DateItem
 * @brief QTableWidgetItem that compares rows based on `Qt::UserRole` timestamp.
 *
 * The item stores a timestamp in `Qt::UserRole` and implements a custom
 * `<` operator so table sorting uses the stored numeric timestamp value.
 */
class DateItem : public QTableWidgetItem {
public:
  using QTableWidgetItem::QTableWidgetItem;

  bool operator<(const QTableWidgetItem &other) const override {
    return data(Qt::UserRole).toLongLong() < other.data(Qt::UserRole).toLongLong();
  }
};

#endif // JS8CALL_DATETABLEWIDGETITEM_H
