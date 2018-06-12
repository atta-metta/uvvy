#include "QmlWindow.h"
#include "WindowManager.h"

#include <QtQml/QQmlContext>
#include <QDebug>

namespace gui {

QmlWindow::QmlWindow(QString qmlFile, WindowManager* m)
    : manager_{m}
    , component_{&m->engine()}
{
    auto c = manager_->engine().rootContext();
    component_.loadUrl(QUrl(qmlFile));

    if (!component_.isReady() ) {
        qFatal("%s", component_.errorString().toUtf8().constData());
    }

    QObject *topLevel = component_.create(c);
    window_ = qobject_cast<QQuickWindow*>(topLevel);

    QSurfaceFormat surfaceFormat = window_->requestedFormat();
    window_->setFormat(surfaceFormat);

    QObject::connect(window_, SIGNAL(closing(QQuickCloseEvent*)), this, SIGNAL(closing()));
}

QmlWindow::~QmlWindow()
{
    /// @todo Need to normally delete the window_;
    hide();
}

}
