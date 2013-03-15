/*
    This file is part of the X2 project from Ofi Labs.

    Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGui>
#include <QtNetwork>
#include <QtWebKit>

#include <iostream>

class Tracker: public QNetworkAccessManager
{
    Q_OBJECT

private:
    int m_replyCounter;
    QHash<QNetworkReply*, int> m_replyHash;
    QTime m_ticker;
    int m_sequence;

public:
    Tracker(QObject *parent = 0)
        : QNetworkAccessManager(parent)
        , m_replyCounter(1)
        , m_sequence(1) {
        m_ticker.start();
        std::cout << "<html isdump=true>" << std::endl;
        std::cout << "<body>" << std::endl;
        std::cout << "<p>Speed Trace <span id=info>(CTRL+S to save)</span></p>" << std::endl;
        std::cout << "<div style=\"display:none\" id=\"traceData\" version=\"0.13\">" << std::endl;
    }

    ~Tracker() {
        std::cout << "</div>" << std::endl;
        std::cout << "</body>" << std::endl;
        std::cout << "</html>" << std::endl;
        QTimer::singleShot(0, QApplication::instance(), SLOT(quit()));
    }

protected:

    const char *toString(Operation op) const {
        const char *str = 0;
        switch (op) {
        case HeadOperation:
            str = "HEAD";
            break;
        case GetOperation:
            str = "GET";
            break;
        case PutOperation:
            str = "PUT";
            break;
        case PostOperation:
            str = "POST";
            break;
        default:
            str = "?";
            break;
        }
        return str;
    }

    // NETWORK_RESOURCE_START
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                 QIODevice *outgoingData) {
        QNetworkReply *reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
        std::cout << "{\"type\":12,";
        std::cout << "\"data\":{";
        std::cout << "\"identifier\":" << m_replyCounter << ",";
        std::cout << "\"url\": \"" << qPrintable(reply->url().toString()) << "\",";
        std::cout << "\"requestMethod\":\"" << toString(op) << "\",";
        std::cout << "\"isMainResource\": false},";
        std::cout << "\"time\":" << m_ticker.elapsed() << ",";
        std::cout << "\"sequence\":" << m_sequence++;
        std::cout << "}" << std::endl;
        m_replyHash[reply] = m_replyCounter++;
        connect(reply, SIGNAL(readyRead()), this, SLOT(startReply()));
        connect(reply, SIGNAL(finished()), this, SLOT(finishReply()));
        return reply;
    }

public slots:
    void finalize() {
        // extra delay for potential last-minute async requests
        QTimer::singleShot(3000, this, SLOT(deleteLater()));
    }

private slots:

    // NETWORK_RESOURCE_RESPONSE
    void startReply() {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
        int contentLength = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
        QString mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (m_replyHash.contains(reply)) {
            std::cout << "{\"type\":13,";
            std::cout << "\"data\":{";
            std::cout << "\"identifier\":" << m_replyHash[reply] << ",";
            std::cout << "\"statusCode\":\"" << statusCode << "\",";
            std::cout << "\"expectedContentLength\": " << contentLength << ",";
            std::cout << "\"mimeType\":\"" << qPrintable(mimeType) << "\"},";
            std::cout << "\"time\":" << m_ticker.elapsed() << ",";
            std::cout << "\"sequence\":" << m_sequence++;
            std::cout << "}" << std::endl;
        }
    }

    // NETWORK_RESOURCE_FINISH
    void finishReply() {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
        if (m_replyHash.contains(reply)) {
            std::cout << "{\"type\":14,";
            std::cout << "\"data\":{";
            std::cout << "\"identifier\":" << m_replyHash[reply] << ",";
            std::cout << "\"didFail\":";
            if (reply->error() == QNetworkReply::NoError)
                std::cout << "false";
            else
                std::cout << "true";
            std::cout << "\"time\":" << m_ticker.elapsed() << ",";
            std::cout << "\"sequence\":" << m_sequence++;
            std::cout << "}" << std::endl;
            m_replyHash.remove(reply);
        }
    }
};

#include "tracenet.moc"

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "tracenet URL" << std::endl << std::endl;
        return 0;
    }

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
    QUrl url = QUrl::fromUserInput(argv[1]);
#else
    QUrl url = QString::fromLatin1(argv[1]);
    if (!url.isValid())
        url = url.toString().prepend("http://");
#endif
    if (!url.isValid()) {
        std::cerr << "Invalid URL: " << argv[1] << std::endl << std::endl;
        return 0;
    }

    QApplication app(argc, argv);

    Tracker *tracker = new Tracker;
    QWebPage page;
    page.setNetworkAccessManager(tracker);
    QObject::connect(&page, SIGNAL(loadFinished(bool)),
                     tracker, SLOT(finalize()));
    page.mainFrame()->load(url);

    return app.exec();
}
