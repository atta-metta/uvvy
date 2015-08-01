#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QLocale>
#include <QSettings>
#include <QtDebug>
#include "chat.h"
#include "chathistory.h"
#include "main.h"
#include "share.h"
#include "view.h"
#include "logarea.h"
#include "peer.h"
#include "stream.h"
#include "xdr.h"
#include "env.h"

using namespace SST;

//=====================================================================================================================
// ChatDialog
//=====================================================================================================================

//QHash<PeerId, ChatDialog*> ChatDialog::chathash;

ChatDialog::ChatDialog(const SST::PeerId &otherid, const QString &othername, Stream *strm)
    : QDialog(NULL)
    , otherid(otherid)
    , othername(othername)
    , stream(NULL)
    , history(NULL)
{
    setWindowTitle(tr("MettaNode chat with %0").arg(othername));
    setAcceptDrops(true);

    settings->beginGroup("ChatWindows/"+othername);
    move(settings->value("pos", QPoint(100, 100)).toPoint());
    resize(settings->value("size", QSize(400, 500)).toSize());
    settings->endGroup();

    logwidget = new QWidget(this);
    loglayout = new QGridLayout();
    loglayout->setColumnStretch(1, 1);
    logwidget->setLayout(loglayout);

    logview = new LogArea(this);
    logview->setWidget(logwidget);
    logview->setWidgetResizable(true);

    textentry = new QLineEdit(this);
    connect(textentry, SIGNAL(returnPressed()),
        this, SLOT(sendTextLine()));

    button = new QPushButton(this);
    button->setText(tr("Send"));
    connect(button, SIGNAL(clicked()),
        this, SLOT(sendTextLine()));

    QPushButton* hbutton = new QPushButton(this);
    hbutton->setText(tr("History"));
    connect(hbutton, SIGNAL(clicked()),
        this, SLOT(loadHistory()));

    QHBoxLayout *inputl = new QHBoxLayout;
    inputl->setContentsMargins(0,0,0,0);
    inputl->setSpacing(1);
    inputl->addWidget(textentry);
    inputl->addWidget(button);
    inputl->addWidget(hbutton);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(1);
    layout->addWidget(logview);
    layout->addLayout(inputl);
    setLayout(layout);

    if (!strm) {
        strm = new Stream(ssthost, this);
        strm->connectTo(otherid, "metta:Chat", "NodeChat");
        textentry->setText(tr("Contacting %0...").arg(othername));
        textentry->setReadOnly(true);
        button->setEnabled(false);
        connect(strm, SIGNAL(linkUp()),
            this, SLOT(connected()));
    }
    connect(strm, SIGNAL(readyReadMessage()),
        this, SLOT(readyReadMessage()));
    connect(strm, SIGNAL(error(const QString &)),
        this, SLOT(streamError(const QString &)));
    stream = strm;

    history = new ChatHistory(otherid, this);
}

ChatDialog::~ChatDialog()
{
    qDebug() << "~ChatDialog";
}

void ChatDialog::closeEvent(QCloseEvent *event)
{
    settings->beginGroup("ChatWindows/"+othername);
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();

    // Close our window as usual
    QDialog::closeEvent(event);

    if (!stream->isConnected() || stream->atEnd())
        deleteLater();  // We can go away completely

    // Send our EOF but hang around to make sure our data gets there
    stream->shutdown(Stream::Write);
}

void ChatDialog::connected()
{
    qDebug() << "ChatDialog: connected";

    // XXX only if not already enabled...
    textentry->clear();
    textentry->setReadOnly(false);
    button->setEnabled(true);
}

void ChatDialog::streamError(const QString &err)
{
    qDebug() << "ChatDialog: streamError" << err;

    if (!isVisible())
        deleteLater();

    textentry->setReadOnly(true);
    button->setEnabled(false);
    textentry->setText(tr("Chat connection to %0 failed") .arg(othername));
    // XX make err details available somehow - write in the chat window
}

void ChatDialog::sendTextLine()
{
    if (!active())
        return;

    QString text = textentry->text();
    if (text.isEmpty())
        return;

    // Add the new text entry to our own log
    addText("Me", text);

    // Send it to our partner too
    QByteArray msg;
    XdrStream ws(&msg, QIODevice::WriteOnly);
    ws << (qint32)Text << text;
    qint64 actsize = stream->writeMessage(msg);
    Q_ASSERT(actsize == msg.size());

    history->insertHistoryLine("Me", QDateTime::currentDateTime(), text);

    // Get ready for another line of text...
    textentry->clear();
}

void ChatDialog::addText(const QString &source, const QString &text)
{
    // XX auto-format text into HTML...
    QLabel *disp = new QLabel(text, logwidget);
    disp->setWordWrap(true);

    // Add the new entry to our log
    addEntry(source, disp);
}

void ChatDialog::addFiles(const QString &source, const QList<FileInfo> &files)
{
    if (files.size() == 0) {
        qDebug() << "ChatDialog::addFiles: no files";
        return;
    }

    // Build a widget containing viewers to browse these files
    QFrame *wid = new QFrame(logwidget);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(5);
    layout->setSpacing(5);
    for (int i = 0; i < files.size(); i++) {
        Viewer *view = new Viewer(wid, files[i]);
        layout->addWidget(view);
    }
    wid->setLayout(layout);
    wid->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    addEntry(source, wid);
}


void ChatDialog::addEntry(QString source, QWidget *widget)
{
    Q_ASSERT(widget->parent() == logwidget);

    // Create a label widget to display the source of this entry.
    QLabel *label = new QLabel(source, logwidget);
    QFont labelfont = label->font();
    labelfont.setBold(true);
    label->setFont(labelfont);

    // Add the label and entry widget to the log.
    int itemno = entries.size();
    labels.append(label);
    entries.append(widget);

    // Add them to the log's layout.
    loglayout->addWidget(label, itemno, 0);
    loglayout->addWidget(widget, itemno, 1);
    loglayout->setRowStretch(itemno, 0);
    loglayout->setRowStretch(itemno+1, 1);
}

// Find the file URLs, if any, in the given MIME data,
// and return just the pathname components.
static QStringList files(const QMimeData *data)
{
    if (!data->hasUrls())
        return QStringList();

    QList<QUrl> urls = data->urls();
    if (urls.isEmpty())
        return QStringList();

    QStringList l;
    foreach (QUrl url, urls) {
        if (url.scheme().toLower() != "file")
            continue;   // Not a local file

        qDebug() << "URL" << url.toString();
        QString path = url.path();
#ifdef WIN32
        // Get rid of the leading slash before the drive letter.
        if (path.size() >= 3 and path[0] == '/' and path[2] == ':')
            path = path.mid(1);
#endif
        l.append(path);
    }
    return l;
}

void ChatDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (!active())
        return QDialog::dragEnterEvent(event);  // reject

    QStringList fl = files(event->mimeData());
    if (fl.isEmpty())
        return QDialog::dragEnterEvent(event);  // reject

    event->acceptProposedAction();
    QDialog::dragEnterEvent(event);
}

void ChatDialog::dropEvent(QDropEvent *event)
{
    if (!active())
        return QDialog::dropEvent(event);   // reject

    QStringList fl = files(event->mimeData());
    if (fl.isEmpty())
        return QDialog::dropEvent(event);   // reject

    event->acceptProposedAction();
    QDialog::dropEvent(event);

    (void)new ChatScanner(this, fl);
}

void ChatDialog::sendFiles(const QList<FileInfo> &files)
{
    if (!active())
        return;

    // Add a browse widget for these files to our own log
    addFiles("Me", files);

    // Send the file keys to our partner
    QByteArray msg;
    XdrStream ws(&msg, QIODevice::WriteOnly);
    qint32 nfiles = files.size();
    ws << (qint32)Files << nfiles;
    for (int i = 0; i < nfiles; i++)
        ws << files[i];
    qint64 actsize = stream->writeMessage(msg);
    Q_ASSERT(actsize == msg.size());
}

ChatDialog *ChatDialog::open(const SST::PeerId &id, const QString &name)
{
    Q_ASSERT(!id.getId().isEmpty());

//  ChatDialog *&dlg = chathash[id];
//  if (dlg == NULL)
//      dlg = new ChatDialog(id, name);
    ChatDialog *dlg = new ChatDialog(id, name);

    dlg->show();
    return dlg;
}

void ChatDialog::readyReadMessage()
{
    while (stream->hasPendingMessages()) {
        QByteArray msg(stream->readMessage());

        XdrStream rs(&msg, QIODevice::ReadOnly);
        qint32 code;
        rs >> code;
        if (rs.status() != rs.Ok) {
            qDebug() << "ChatDialog: received runt message";
            continue;
        }

        switch ((MsgCode)code) {
        case Text: {
            QString text;
            rs >> text;
            if (rs.status() != rs.Ok) {
                qDebug() << "Chat: invalid text message";
                break;
            }
            addText(othername, text);
            history->insertHistoryLine(othername, QDateTime::currentDateTime(), text); // @todo timestamp should come from network??
            break; }
        case Files: {
            qint32 nfiles;
            rs >> nfiles;
            if (rs.status() != rs.Ok || nfiles <= 0) {
                qDebug() << "Chat: invalid files message";
                break;
            }
            QList<FileInfo> files;
            for (int i = 0; i < nfiles; i++) {
                FileInfo fi;
                rs >> fi;
                if (rs.status() != rs.Ok) {
                    qDebug() << "Chat: error decoding file"
                        << i;
                    break;
                }
                files.append(fi);
            }
            addFiles(othername, files);
            break; }
        default:
            qDebug() << "ChatDialog: unknown message code" << code;
            break;
        }
    }

    if (stream->atEnd()) {
        if (!isVisible())
            deleteLater();

        textentry->setReadOnly(true);
        button->setEnabled(false);
        textentry->setText(tr("Chat session closed by %0").arg(othername));
    }
}

void ChatDialog::loadHistory()
{
}

//=====================================================================================================================
// ChatServer
//=====================================================================================================================

ChatServer::ChatServer(QObject *parent)
:   StreamServer(ssthost, parent)
{
    listen("metta:Chat", "Instant Messaging",
        "NodeChat", "MettaNode Chat Protocol");

    connect(this, SIGNAL(newConnection()),
        this, SLOT(incoming()));
}

void ChatServer::incoming()
{
    while (1) {
        Stream *strm = accept();
        if (!strm)
            return;

        PeerId id = strm->remoteHostId();
        QString name = friends->name(id);
        if (name.isEmpty())
            name = tr("unknown host %0").arg(QString(id.toString()));

        qDebug() << "ChatServer: accepting incoming stream from"<< id << name;

        ChatDialog *dlg = new ChatDialog(id, name, strm);
        dlg->show();

        // Check for more queued incoming connections
    }
}

//=====================================================================================================================
// ChatScanner
//=====================================================================================================================

ChatScanner::ChatScanner(ChatDialog *parent, const QStringList &files)
    : QProgressDialog(parent)
    , chat(parent)
{
    connect(parent, SIGNAL(finished(int)), this, SLOT(scanCanceled()));
    connect(this, SIGNAL(canceled()), this, SLOT(scanCanceled()));

    int nfiles = files.size();
    for (int i = 0; i < nfiles; i++) {
        Scan *scan = new Scan(this, files.at(i));
        scans.append(scan);

        connect(scan, SIGNAL(statusChanged()),
            this, SLOT(scanProgress()));
        connect(scan, SIGNAL(progressChanged(float)),
            this, SLOT(scanProgress()));
    }

    setRange(0, 1000000);

    // Start the scans in a separate pass at the end;
    // otherwise if they complete immediately
    // they might think the entire file set is done scanning.
    for (int i = 0; i < nfiles; i++)
        scans.at(i)->start();
    scanProgress();
}

void ChatScanner::scanProgress()
{
    if (scans.isEmpty())
        return;

    int nfiles = 0, nerrors = 0;
    qint64 gotbytes = 0, totbytes = 0;
    bool alldone = true;
    for (int i = 0; i < scans.size(); i++) {
        Scan *scan = scans.at(i);
        nfiles += scan->numFiles();
        nerrors += scan->numErrors();
        gotbytes += scan->numBytes();
        totbytes += scan->totalBytes();
        if (!scan->isDone())
            alldone = false;
    }
    qDebug() << "ChatScanner:" << gotbytes << "of" << totbytes;

    if (alldone) {
        QList<FileInfo> fil;
        for (int i = 0; i < scans.size(); i++)
            fil.append(scans.at(i)->fileInfo());
        chat->sendFiles(fil);
        scans.clear();
        return deleteLater();
    }

    QString text = tr("Scanning: %0 bytes in %1 files")
            .arg(QLocale::system().toString(gotbytes))
            .arg(QLocale::system().toString(nfiles));
    if (nerrors == 1)
        text += " (1 error)";
    else if (nerrors > 1)
        text += tr(" (%0 errors)")
            .arg(QLocale::system().toString(nerrors));
    setLabelText(text);

    float ratio = (float)gotbytes / (float)totbytes;
    setValue((int)(1000000 * ratio));
}

void ChatScanner::scanCanceled()
{
    deleteLater();
}

