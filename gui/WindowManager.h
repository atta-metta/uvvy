#pragma once

#include <QObject>
#include <QtQml/QQmlEngine>

namespace gui {

class QmlWindow;
class UserInfo;
class Root;

/**
 * @class WindowManager
 * @brief Main class to handle windows and interaction between them.
 *
 *        WindowManager is central class to manage windows creation deletion
 *        and interaction between them. It is exported in qml context and 
 *        can be accessible from qml by @c windowManager object.
 */
class WindowManager : public QObject
{
    Q_OBJECT

public:
    WindowManager(Root* r);
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    /// @brief Access to engine.
    inline QQmlEngine& engine()
    {
        return engine_;
    }

    /// @brief Access to qml context.
    inline QQmlContext* qmlContext()
    {
        return engine().rootContext();
    }

    /// @brief Access to root.
    inline Root* root()
    {
        return root_;
    }

    /// @brief Opens registration window.
    Q_INVOKABLE void openRegistrationWindow();

    /// @brief Closes registration window.
    Q_INVOKABLE void closeRegistrationWindow();
    
    /// @brief Logs in given user and opens main window with user's contact list.
    /// @param u User to log in.
    void login(UserInfo* u);

    /// @brief Starts audio call between two users.
    /// @param u1 User who calls.
    /// @param u2 User to call.
    void startCall(UserInfo* u1, UserInfo* u2);

    /// @brief Starts chat between two users.
    /// @param u1 User starting chat.
    /// @param u2 User to chat with.
    void startChat(UserInfo* u1, UserInfo* u2);

private:
    QQmlEngine engine_;
    QmlWindow* mainWindow_;
    QmlWindow* registrationWindow_;
    Root* root_;
};

}
