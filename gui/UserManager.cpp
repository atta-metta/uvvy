#include "UserManager.h"

#include "UserInfo.h"
#include "exceptions.h"

namespace gui {

UserManager* UserManager::login(QString u, QString p)
{
    return new UserManager{u, p};
}

UserManager::UserManager(QString u, QString p)
    : user_{new UserInfo}
{
    user_->read();
    contacts_["AAAA"] = new UserInfo(*user_);
    contacts_["BBBB"] = new UserInfo(*user_);
}

UserInfo* UserManager::user()
{
    return user_;
}

UserInfo* UserManager::userById(QString id)
{
    auto i = contacts_.find(id);
    if (i == contacts_.end()) {
        throw invalid_user{id};
    }
    return i->second;
}

void UserManager::addContact(UserInfo* u)
{
    contacts_["cccc"] = u;
    emit userAdded(u);
}

}
