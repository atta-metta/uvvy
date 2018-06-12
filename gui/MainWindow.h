#pragma once

#include "QmlWindow.h"

namespace gui {

class UserInfo;

class MainWindow : public QmlWindow
{
    Q_OBJECT
public:
    MainWindow(WindowManager* m, UserInfo* u);
    ~MainWindow();

public slots:
    void updateContactList(UserInfo*);

private:
    QList<QObject*> contactList_;
};

}
