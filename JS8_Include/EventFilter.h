/**
 * @file EventFilter.h
 * @brief Custom event filters for Qt applications.
 *
 * This header defines several custom event filter classes that can be used
 * to handle specific events in Qt applications, such as focus out, key presses,
 * and mouse button events. Each filter class allows the user to specify a
 * callback function that will be invoked when the corresponding event occurs.
 */

#ifndef EVENTFILTER_HPP__
#define EVENTFILTER_HPP__

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>

#include <functional>

namespace EventFilter {
/**
 * @class FocusOut
 * @brief Event filter that triggers a callback on focus out events.
 *
 * This filter can be installed on a QObject to invoke a user-defined
 * callback function whenever the object loses focus.
 */
class FocusOut final : public QObject {
  public:
    using Filter = std::function<void()>;

    FocusOut(Filter filter, QObject *parent = nullptr)
        : QObject{parent}, filter_{filter} {}

    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::FocusOut)
            filter_();
        return false;
    }

  private:
    Filter filter_;
};

/**
 * @class FocusIn
 * @brief Event filter that triggers a callback on focus in events.
 *
 * This filter can be installed on a QObject to invoke a user-defined
 * callback function whenever the object gains focus.
 */
class EscapeKeyPress final : public QObject {
  public:
    using Filter = std::function<bool(QKeyEvent *)>;

    EscapeKeyPress(Filter filter, QObject *parent = nullptr)
        : QObject{parent}, filter_{filter} {}

    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            if (auto const keyEvent = static_cast<QKeyEvent *>(event);
                keyEvent->key() == Qt::Key_Escape) {
                return filter_(keyEvent);
            }
        }
        return false;
    }

  private:
    Filter filter_;
};

/**
 * @class EnterKeyPress
 * @brief Event filter that triggers a callback on Enter/Return key presses.
 *
 * This filter can be installed on a QObject to invoke a user-defined
 * callback function whenever the Enter or Return key is pressed.
 */
class EnterKeyPress final : public QObject {
  public:
    using Filter = std::function<bool(QKeyEvent *)>;

    EnterKeyPress(Filter filter, QObject *parent = nullptr)
        : QObject{parent}, filter_{filter} {}

    bool eventFilter(QObject *, QEvent *const event) override {
        if (event->type() == QEvent::KeyPress) {
            if (auto const keyEvent = static_cast<QKeyEvent *>(event);
                keyEvent->key() == Qt::Key_Enter ||
                keyEvent->key() == Qt::Key_Return) {
                return filter_(keyEvent);
            }
        }
        return false;
    }

  private:
    Filter filter_;
};

/**
 * @class MouseButtonPress
 * @brief Event filter that triggers a callback on mouse button press events.
 *
 * This filter can be installed on a QObject to invoke a user-defined
 * callback function whenever a mouse button is pressed.
 */
class MouseButtonPress final : public QObject {
  public:
    using Filter = std::function<bool(QMouseEvent *)>;

    MouseButtonPress(Filter filter, QObject *parent = nullptr)
        : QObject{parent}, filter_{filter} {}

    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            return filter_(static_cast<QMouseEvent *>(event));
        }
        return false;
    }

  private:
    Filter filter_;
};

/**
 * @class MouseButtonDblClick
 * @brief Event filter that triggers a callback on mouse button double-click events.
 *
 * This filter can be installed on a QObject to invoke a user-defined
 * callback function whenever a mouse button is double-clicked.
 */
class MouseButtonDblClick final : public QObject {
  public:
    using Filter = std::function<bool(QMouseEvent *)>;

    MouseButtonDblClick(Filter filter, QObject *parent = nullptr)
        : QObject{parent}, filter_{filter} {}

    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonDblClick) {
            return filter_(static_cast<QMouseEvent *>(event));
        }
        return false;
    }

  private:
    Filter filter_;
};
} // namespace EventFilter

#endif
