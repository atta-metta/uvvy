#include "Root.h"

#include "AccountManager.h"
#include "UserInfo.h"
#include "UserManager.h"
#include "WindowManager.h"

#include <QQmlContext>

namespace gui {

Root::Root()
    : windowManager_{new WindowManager{this}}
    , accountManager_{new AccountManager{this}}
{
    windowManager_->qmlContext()->setContextProperty("root", this);
}

void Root::login(const QString& u, const QString& p)
{
    accountManager_->login(u, p);
    auto s = accountManager_->mainUser()->user();
    windowManager_->login(s);
}

void Root::startCall(const QString& id)
{
    auto u1 = accountManager_->mainUser()->user();
    auto u2 = accountManager_->mainUser()->userById(id);
    windowManager_->startCall(u1, u2);
}

void Root::startChat(const QString& id)
{
    auto u1 = accountManager_->mainUser()->user();
    auto u2 = accountManager_->mainUser()->userById(id);
    windowManager_->startChat(u1, u2);
}

void Root::exit() const
{
    std::exit(0);
}

}
