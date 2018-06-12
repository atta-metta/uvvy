#include "AccountManager.h"

#include "Root.h"
#include "UserInfo.h"
#include "UserManager.h"
#include "WindowManager.h"
#include "exceptions.h"

#include <QQmlContext>

#include <fstream>
#include <iostream>

namespace gui {

AccountManager::AccountManager(Root* r)
    : root_{r}
{
    root_->windowManager().qmlContext()->setContextProperty("accountManager", this);
}

void AccountManager::login(QString u, QString p)
{
    /// Get user info by username and password.
    mainUser_ = UserManager::login(u, p);
}

UserManager* AccountManager::mainUser() {
    if (mainUser_ == nullptr) {
        throw not_logged_in{};
    }
    return mainUser_;
}

void AccountManager::registerUser(QString u, QString p, QString e, QString f,
        QString l, QString c, QString a)
{
    std::string h = std::getenv("HOME");
    std::ofstream ff(h + "/.uvvy/users", std::ios_base::ate | std::ios_base::out);
    ff << u.toStdString() << " ";
    ff << p.toStdString() << " ";
    ff << e.toStdString() << " ";
    ff << f.toStdString() << " ";
    ff << l.toStdString() << " ";
    ff << c.toStdString() << " ";
    ff << a.toStdString() << " " << std::endl;
}

QList<QObject*> AccountManager::findContact(QString u)
{
    QList<QObject*> r;
    UserInfo* i = new UserInfo();
    i->read();

    r.append(i);
    auto c = root_->windowManager().qmlContext();
    c->setContextProperty("foundContactsModel", QVariant::fromValue(r));
    return r;
}

void AccountManager::contactRequest(QObject* u)
{
    mainUser_->addContact(static_cast<UserInfo*>(u));
}

}
