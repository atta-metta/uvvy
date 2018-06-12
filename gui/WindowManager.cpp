#include "WindowManager.h"

#include "AccountManager.h"
#include "MainWindow.h"
#include "QmlWindow.h"
#include "Root.h"
#include "UserInfo.h"
#include "UserManager.h"

#include <QtQml/QQmlContext>

#include <iostream>

namespace gui {

WindowManager::WindowManager(Root* r)
    : mainWindow_{new QmlWindow{"quick/LoginScreen.qml", this}}
    , root_{r}
{
    qmlContext()->setContextProperty("windowManager", this);
    QObject::connect(mainWindow_, SIGNAL(closing()), root_, SLOT(exit()));
    mainWindow_->show();
}

void WindowManager::openRegistrationWindow()
{
    registrationWindow_ = new QmlWindow{"quick/RegistrationWindow.qml", this};
    QObject::connect(registrationWindow_, SIGNAL(closing()), this, SLOT(closeRegistrationWindow()));
    registrationWindow_->show();
    mainWindow_->hide();
}

void WindowManager::closeRegistrationWindow()
{
    registrationWindow_->hide();
    mainWindow_->show();
}

void WindowManager::login(UserInfo* u)
{
    auto s = new MainWindow{this, u};
    s->show();
    delete mainWindow_;
    mainWindow_ = s;
    QObject::connect(mainWindow_, SIGNAL(closing()), root_, SLOT(exit()));
    QObject::connect(root()->accountManager().mainUser(), SIGNAL(userAdded(UserInfo*)),
            s, SLOT(updateContactList(UserInfo*)));
}

void WindowManager::startCall(UserInfo* u1, UserInfo* u2)
{
    std::cout << u1->firstName().toStdString() << " " << u1->lastName().toStdString() << " calling to " <<
        u2->firstName().toStdString() << " " << u2->lastName().toStdString() << std::endl;
}

void WindowManager::startChat(UserInfo* u1, UserInfo* u2)
{
    std::cout << u1->firstName().toStdString() << " " << u1->lastName().toStdString() << " chatting to " <<
        u2->firstName().toStdString() << " " << u2->lastName().toStdString() << std::endl;
}

}
