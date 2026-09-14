/**
 * @file SemiSortableHeader.h
 * @brief Header view that selectively disables sorting on specified columns.
 */

#ifndef JS8CALL_SEMISORTABLEHEADER_H
#define JS8CALL_SEMISORTABLEHEADER_H

#include <QHeaderView>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QSet>
#include <QStyleOptionHeader>
#include <QTableWidget>
#include <QVariant>


/**
 * @class SemiSortableHeader
 * @brief QHeaderView derivative that allows selective disabling of sorting.
 *
 * The header can be attached to a `QTableWidget` and configured with a set of
 * non-sortable columns. Clicks on non-sortable columns are ignored and the
 * header is painted without a sort indicator for those columns.
 */
class SemiSortableHeader : public QHeaderView {

public:
  /**
   * @brief Construct the header with the given orientation.
   * @param ori Header orientation (Horizontal/Vertical).
   * @param parent Optional parent widget.
   */
  explicit SemiSortableHeader(Qt::Orientation ori, QWidget *parent = nullptr)
      : QHeaderView(ori, parent) {
    setSectionsClickable(true);
    setSortIndicatorShown(true);
    setSortIndicatorClearable(false);
    setMouseTracking(true);
  }

  /**
   * @brief Attach this header to a QTableWidget and initialize sorting state.
   * @param table Target table to attach to.
   */
  void attachTo(QTableWidget *table) {
    table_ = table;
    table_->setHorizontalHeader(this);

    // Keep sorting enabled so the style draws the indicator
    table_->setSortingEnabled(true);

    // Initialize to a sensible default: first sortable column, descending
    sortCol_ = firstSortableSection();
    sortOrder_ = Qt::DescendingOrder;

    if (sortCol_ >= 0) {
      QSignalBlocker b(this);
      setSortIndicator(sortCol_, sortOrder_);
    }

    connect(this, &QHeaderView::sectionClicked, this,
            &SemiSortableHeader::onSectionClicked, Qt::UniqueConnection);
  }

  /**
   * @brief Set the set of non-sortable columns.
   * @param cols Set of column indexes that should not be sortable.
   */
  void setNonSortableColumns(const QSet<int> &cols) {
    nonSortable_ = cols;
    normalizeSortIfNeeded();
    viewport()->update();
  }

  /**
   * @brief Mark a specific column as non-sortable.
   */
  void addNonSortableColumn(int col) {
    if (col >= 0) {
      nonSortable_.insert(col);
      normalizeSortIfNeeded();
      viewport()->update();
    }
  }

  /**
   * @brief Remove non-sortable marking from a column.
   */
  void removeNonSortableColumn(int col) {
    nonSortable_.remove(col);
    normalizeSortIfNeeded();
    viewport()->update();
  }

  /**
   * @brief Check whether a column is sortable.
   * @return true if the column is sortable.
   */
  bool isSortableColumn(int col) const {
    return col >= 0 && !nonSortable_.contains(col);
  }

private slots:
  /**
   * @brief Handle clicks on sections and perform toggled sorting.
   * @param column Clicked column index.
   */
  void onSectionClicked(int column) {
    if (!table_)
      return;
    if (!isSortableColumn(column))
      return;

    if (column == sortCol_) {
      sortOrder_ = (sortOrder_ == Qt::AscendingOrder) ? Qt::DescendingOrder
                                                      : Qt::AscendingOrder;
    } else {
      sortCol_ = column;
      sortOrder_ = Qt::AscendingOrder;
    }

    // Update indicator without re-triggering click logic
    {
      QSignalBlocker b(this);
      setSortIndicator(sortCol_, sortOrder_);
    }

    // Prevent Qt's built-in sorting from running while we sort
    table_->setSortingEnabled(false);
    table_->sortItems(sortCol_, sortOrder_);
    table_->setSortingEnabled(true);
  }

protected:
  /**
   * @brief Custom painting to avoid drawing sort indicators for non-sortable columns.
   */
  void paintSection(QPainter *painter, const QRect &rect,
                    int logicalIndex) const override {
    // Normal columns: keep Qt default painting
    if (isSortableColumn(logicalIndex)) {
      QHeaderView::paintSection(painter, rect, logicalIndex);
      return;
    }

    // Dead columns: custom paint (no hover/pressed, no sort arrow), but keep
    // label
    QStyleOptionHeader opt;
    initStyleOption(&opt);

    opt.rect = rect;
    opt.section = logicalIndex;

    if (auto *m = model()) {
      opt.text = m->headerData(logicalIndex, orientation(), Qt::DisplayRole)
                     .toString();

      const QVariant align =
          m->headerData(logicalIndex, orientation(), Qt::TextAlignmentRole);
      if (align.isValid())
        opt.textAlignment = Qt::Alignment(align.toInt());

      const QVariant deco =
          m->headerData(logicalIndex, orientation(), Qt::DecorationRole);
      if (deco.isValid())
        opt.icon = deco.value<QIcon>();
    }

    // Section position affects borders/separators
    const int v = visualIndex(logicalIndex);
    const int n = count();
    if (n <= 1)
      opt.position = QStyleOptionHeader::OnlyOneSection;
    else if (v == 0)
      opt.position = QStyleOptionHeader::Beginning;
    else if (v == n - 1)
      opt.position = QStyleOptionHeader::End;
    else
      opt.position = QStyleOptionHeader::Middle;

    opt.state &= ~QStyle::State_MouseOver;
    opt.state &= ~QStyle::State_Sunken;

    // Ensure no sort arrow is painted on dead columns
    opt.sortIndicator = QStyleOptionHeader::None;

    style()->drawControl(QStyle::CE_Header, &opt, painter, this);
  }

  /**
   * @brief Intercept mouse presses to ignore clicks on non-sortable columns.
   */
  void mousePressEvent(QMouseEvent *e) override {
    const int col = logicalIndexAt(e->pos());
    if (!isSortableColumn(col)) {
      e->accept(); // swallow: no indicator change, no click signal
      return;
    }
    QHeaderView::mousePressEvent(e);
  }

private:
  int firstSortableSection() const {
    for (int logical = 0; logical < count(); ++logical) {
      if (isSortableColumn(logical))
        return logical;
    }
    return -1; // none sortable
  }

  void normalizeSortIfNeeded() {
    // If current sort column became non-sortable, move to first sortable
    if (sortCol_ >= 0 && !isSortableColumn(sortCol_)) {
      sortCol_ = firstSortableSection();
      sortOrder_ = Qt::DescendingOrder;
      if (sortCol_ >= 0) {
        QSignalBlocker b(this);
        setSortIndicator(sortCol_, sortOrder_);
      } else {
        QSignalBlocker b(this);
        setSortIndicator(-1, Qt::AscendingOrder); // clear indicator
      }
    }
  }

private:
  QPointer<QTableWidget> table_;
  QSet<int> nonSortable_;

  int sortCol_ = -1;
  Qt::SortOrder sortOrder_ = Qt::AscendingOrder;
};

#endif // JS8CALL_SEMISORTABLEHEADER_H
