#pragma once

#include <QObject>

namespace gui {

class UserInfo;
class UserManager;
class Root;

/**
 * @class AccountManager
 * @brief Class to handle account, register and log in user.
 *
 *        AccountManager is central class to handle users and their account.
 *        It provides interface to login user, register user, get contacts of user, etc.
 *        AccountManager single object is exported to qml and can be accessible by
 *        @c accountManager object.
 */
class AccountManager : public QObject
{
    Q_OBJECT

public:
    AccountManager(Root* r);
    AccountManager(const AccountManager&) = delete;
    AccountManager& operator=(const AccountManager&) = delete;

    /// @brief Logs in given user.
    /// @param u Username.
    /// @param p Password.
    void login(QString u, QString p);

    /// @brief Access to main logged in user.
    /// @throw not_logged_in
    UserManager* mainUser();

    /**
     * @brief Registers user with given parameters.
     * @param u Username.
     * @param p Password.
     * @param e Email.
     * @param f First name.
     * @param l Last name.
     * @param c City.
     * @param a Avatar.
     */
    Q_INVOKABLE void registerUser(QString u, QString p, QString e, QString f,
            QString l, QString c, QString a);

    /**
     * @brief Find contact with given credentials.
     * @param u Credential (username, name or email).
     */
    Q_INVOKABLE QList<QObject*> findContact(QString u);

    /**
     * @brief Send contact request to given user.
     * @param u User's base QObject class.
     */
    Q_INVOKABLE void contactRequest(QObject* u);

private:
    Root* root_;
    UserManager* mainUser_;
};

}
