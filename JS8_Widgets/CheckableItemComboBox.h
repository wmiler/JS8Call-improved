/**
 * @file CheckableItemComboBox.h
 * @brief ComboBox supporting checkable items and aggregated display text.
 */

#ifndef CHECKABLE_ITEM_COMBO_BOX_HPP__
#define CHECKABLE_ITEM_COMBO_BOX_HPP__

#include "LazyFillComboBox.h"

#include <QScopedPointer>

class QStandardItemModel;
class QStandardItem;


/**
 * @class CheckableItemComboBox
 * @brief A ComboBox that displays multiple checkable items and summarizes selection.
 *
 * Items added via `addCheckItem()` are represented by `QStandardItem`s with
 * check states; the widget updates its displayed text to reflect selected
 * entries and forwards item press events to update the model state.
 */
class CheckableItemComboBox : public LazyFillComboBox {
    Q_OBJECT

  public:
    /**
     * @brief Construct the widget.
     * @param parent Optional parent widget.
     */
    explicit CheckableItemComboBox(QWidget *parent = nullptr);

    /**
     * @brief Add a checkable item to the combobox model.
     * @param label Visible item label.
     * @param data Associated user data.
     * @param checkState Initial check state for the item.
     * @return The created QStandardItem pointer.
     */
    QStandardItem *addCheckItem(QString const &label, QVariant const &data,
                                Qt::CheckState checkState);

  protected:
    /**
     * @brief Event filter to intercept model and item events.
     */
    bool eventFilter(QObject *, QEvent *) override;

  private:
    /**
     * @brief Update the line-edit text to summarize selected items.
     */
    void update_text();

    Q_SLOT void model_data_changed();
    Q_SLOT void item_pressed(QModelIndex const &);

  private:
    QScopedPointer<QStandardItemModel> model_; /**< Underlying model for checkable items. */
};

#endif
