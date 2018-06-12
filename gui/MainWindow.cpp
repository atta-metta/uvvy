#include "MainWindow.h"
#include "AccountManager.h"
#include "Root.h"
#include "UserInfo.h"
#include "UserManager.h"
#include "WindowManager.h"

#include <QtQml/QQmlContext>

namespace gui {

MainWindow::MainWindow(WindowManager* m, UserInfo* u)
    : QmlWindow{"quick/MainWindow.qml", m}
{
    auto a = manager_->root()->accountManager().mainUser();
    a->foreach_contact([this](UserInfo* u) {
            contactList_.append(u);
            });
    auto c = manager_->qmlContext();
    c->setContextProperty("contactListModel", QVariant::fromValue(contactList_));
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateContactList(UserInfo* u)
{
    contactList_.append(u);
    auto c = manager_->qmlContext();
    c->setContextProperty("contactListModel", QVariant::fromValue(contactList_));
}

}
