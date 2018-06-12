#pragma once

#include <QObject>

#include <string>

namespace gui {

class UserInfo : public QObject
{
    Q_OBJECT

public:
    UserInfo() = default;
    UserInfo(const UserInfo&);
    virtual ~UserInfo() = default;

public:
    Q_PROPERTY(QString firstName READ firstName)
    Q_PROPERTY(QString lastName READ lastName)
    Q_PROPERTY(QString userName READ username)
    Q_PROPERTY(QString email READ email)
    Q_PROPERTY(QString avatarUrl READ avatar)
    Q_PROPERTY(QString host READ host)
    Q_PROPERTY(QString city READ city)
    Q_PROPERTY(QString eid READ eid)
    Q_PROPERTY(QObject* this_ READ self())

public:
    const QString& firstName() const
    {
        return firstName_;
    }

    const QString& lastName() const
    {
        return lastName_;
    }

    const QString& username() const
    {
        return username_;
    }

    const QString& email() const
    {
        return email_;
    }

    const QString& avatar() const
    {
        return avatar_;
    }

    const QString& host() const
    {
        return host_;
    }

    const QString& city() const
    {
        return city_;
    }

    const QString& eid() const
    {
        return eid_;
    }

    QObject* self() {
        return this;
    }

public:
    void read();

    void write() const;

private:
    QString firstName_;
    QString lastName_;
    QString username_;
    QString email_;
    QString avatar_;
    QString host_;
    QString city_;
    QString eid_;
};

}
