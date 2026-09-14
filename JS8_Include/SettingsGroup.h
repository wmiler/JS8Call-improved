/**
 * @file SettingsGroup.h
 * @brief RAII wrapper for QSettings group management.
 *
 * This header defines the SettingsGroup class, which provides a simple RAII
 * mechanism to manage QSettings groups within a scope. It ensures that the
 * group is properly ended when the object goes out of scope.
 *
 * @see QSettings for more information on settings management in Qt.
 */
#ifndef SETTINGS_GROUP_HPP_
#define SETTINGS_GROUP_HPP_

#include <QSettings>
#include <QString>

/**
 * @class SettingsGroup
 * @brief RAII wrapper for QSettings group management.
 *
 * The SettingsGroup class provides a simple RAII mechanism to manage
 * QSettings groups within a scope. It ensures that the group is properly
 * ended when the object goes out of scope, preventing potential issues
 * with unbalanced begin/end group calls.
 *
 */
class SettingsGroup {
  public:
    SettingsGroup(QSettings *settings, QString const &group)
        : settings_{settings} {
        settings_->beginGroup(group);
    }

    SettingsGroup(SettingsGroup const &) = delete;
    SettingsGroup &operator=(SettingsGroup const &) = delete;

    ~SettingsGroup() { settings_->endGroup(); }

  private:
    QSettings *settings_;
};

#endif
