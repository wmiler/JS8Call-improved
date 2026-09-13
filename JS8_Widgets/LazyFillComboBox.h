/**
 * @file LazyFillComboBox.h
 * @brief Lightweight QComboBox that emits signals when the popup is shown/hidden.
 */

#ifndef LAZY_FILL_COMBO_BOX_HPP__
#define LAZY_FILL_COMBO_BOX_HPP__

#include <QComboBox>

class QWidget;


/**
 * @class LazyFillComboBox
 * @brief QComboBox derivative that notifies when its popup is shown or hidden.
 *
 * The widget emits `about_to_show_popup()` before the popup is displayed and
 * `popup_hidden()` after the popup is closed. This is useful for lazy-populating
 * the list contents immediately before display.
 */
class LazyFillComboBox : public QComboBox {
    Q_OBJECT

  public:
    /**
     * @brief Emitted immediately before the popup is shown.
     */
    Q_SIGNAL void about_to_show_popup();

    /**
     * @brief Emitted after the popup has been hidden.
     */
    Q_SIGNAL void popup_hidden();

    /**
     * @brief Construct a LazyFillComboBox.
     * @param parent Optional parent widget.
     */
    explicit LazyFillComboBox(QWidget *parent = nullptr) : QComboBox{parent} {}

    /**
     * @brief Override to emit `about_to_show_popup()` before showing.
     */
    void showPopup() override {
        Q_EMIT about_to_show_popup();
        QComboBox::showPopup();
    }

    /**
     * @brief Override to emit `popup_hidden()` after hiding the popup.
     */
    void hidePopup() override {
        QComboBox::hidePopup();
        Q_EMIT popup_hidden();
    }
};

#endif
