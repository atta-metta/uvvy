#pragma once

#include <QObject>
#include <QString>

namespace gui {

class UserInfo;

class UserManager : public QObject
{
    Q_OBJECT
public:
    UserManager(QString u, QString p);

    static UserManager* login(QString u, QString p);

public:
    UserInfo* user();
    UserInfo* userById(QString id);

    void addContact(UserInfo*);

    template <typename F>
    void foreach_contact(F f)
    {
        for (auto& p : contacts_) {
            f(p.second);
        }
    }

signals:
    void userAdded(UserInfo* u);

private:
    std::map<QString, UserInfo*> contacts_;
    UserInfo* user_;
};

}
