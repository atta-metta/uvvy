#include "UserInfo.h"

namespace gui {

UserInfo::UserInfo(const UserInfo& u)
    : QObject{}
    , firstName_{u.firstName_}
    , lastName_{u.lastName_}
    , username_{u.username_}
    , email_{u.email_}
    , avatar_{u.avatar_}
    , host_{u.host_}
    , city_{u.city_}
    , eid_{u.eid_}
{
}

void UserInfo::read()
{
    firstName_ = "Naaame";
    lastName_ = "Surnaaame";
    username_ = "Usernaaame";
    email_ = "name@example.com";
    avatar_ = "https://camo.githubusercontent.com/3793f6770550a407f10bfa3480d62fe720cb9d7f/68747470733a2f2f7261772e6769746875622e636f6d2f6265726b75732f6d657474612f6d61737465722f646f63732f6d657474612e706e67";
    host_ = "HOST";
    city_ = "Antananarivu";
    eid_ = "IDIDIDIDIDIDID";
}

void UserInfo::write() const
{
}

}
