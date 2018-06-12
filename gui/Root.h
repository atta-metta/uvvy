#pragma once

#include <QObject>

namespace gui {

class AccountManager;
class WindowManager;

/**
 * @class Root
 * @brief Root class for GUI.
 *
 *        Root is singleton class to managing whole GUI system.
 *        The main purpose of Root is to keep window and account managers
 *        and provide interaction between them.
 *        Root is exported to qml context and is accessible by @c root object.
 */
class Root : public QObject
{
    Q_OBJECT

public:
    Root();
    Root(const Root&) = delete;
    Root& operator=(const Root&) = delete;

    /// @brief Logs in given user and opens main window with contacts.
    /// @param u Username.
    /// @param p Password.
    Q_INVOKABLE void login(const QString& u, const QString& p);

    Q_INVOKABLE void startCall(const QString& id);
    Q_INVOKABLE void startChat(const QString& id);

    /// @brief Exits the tool.
    Q_INVOKABLE void exit() const;

public:
    /// @brief Access to account manager.
    inline AccountManager& accountManager()
    {
        return *accountManager_;
    }

    /// @brief Access to window manager.
    inline WindowManager& windowManager()
    {
        return *windowManager_;
    }

private:
    WindowManager* windowManager_;
    AccountManager* accountManager_;
};

}
