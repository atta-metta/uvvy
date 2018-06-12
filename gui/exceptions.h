#pragma once

namespace gui {

class base_exception
{
public:
    virtual QString what() const = 0;
};

class not_logged_in : public base_exception
{
public:
    virtual QString what() const override
    {
        return QString("User not logged in yet");
    }
};

class invalid_user : public base_exception
{
public:
    invalid_user(QString id)
        : id_{id}
    {
    }

    virtual QString what() const override
    {
        return QString("Invalid user ") + id_;
    }

private:
    QString id_;
};

}
