#pragma once

#include <QtQml/QQmlComponent>
#include <QtQuick/QQuickWindow>

namespace gui {

class WindowManager;

/**
 * @class QmlWindow
 * @brief Base class to create top level qml windows.
 *
 *        QmlWindow represents single window created in qml.
 *        It gets qml file and constructs window based on it.
 */
class QmlWindow : public QObject
{
    Q_OBJECT

protected:
    WindowManager *manager_{nullptr};
    QQmlComponent component_;
    QQuickWindow* window_{nullptr};

signals:
    void closing();

public:
    QmlWindow(QString qmlFile, WindowManager* m);
    virtual ~QmlWindow();

    inline void show() {
        window_->show();
    }

    inline void hide() {
        window_->hide();
    }
};

}
