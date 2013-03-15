/*
    This file is part of the Ofi Labs X2 project.

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

#include <QtNetwork>
#include <QtSql>

class NetSpiegel: public QObject
{
    Q_OBJECT

private:
    QTcpServer cachingProxy;
    QTcpServer mirrorProxy;
    QNetworkAccessManager networkManager;
    QSqlDatabase database;

public:

    NetSpiegel(QObject *parent = 0)
        : QObject(parent)
    {
        cachingProxy.listen(QHostAddress::Any, 8080);
        mirrorProxy.listen(QHostAddress::Any, 8081);
        connect(&cachingProxy, SIGNAL(newConnection()), this, SLOT(manageQuery()));
        connect(&mirrorProxy, SIGNAL(newConnection()), this, SLOT(manageQuery()));
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("mirror.db");
        if (!database.open())
            qFatal("Can not process %s", qPrintable(database.databaseName()));
        QSqlQuery query(database);
        if (!query.exec("CREATE TABLE IF NOT EXISTS "
                        "responses(url blob NOT NULL UNIQUE, "
                        "header blob, data blob)"))
            qFatal("Can not initialize table in %s", qPrintable(database.databaseName()));
    }

private slots:

    void manageQuery() {
        QTcpServer *server = qobject_cast<QTcpServer*>(sender());
        QTcpSocket *socket = server->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), this, SLOT(processQuery()));
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    }

    void processQuery() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        QList<QByteArray> queryLines = socket->readAll().split('\n');
        QList<QByteArray> entries = queryLines.value(0).split(' ');
        QByteArray method = entries.value(0);
        QByteArray address = entries.value(1);

        if (method != "GET") {
            qWarning() << "Unsupported method" << method << "for" << address;
            socket->disconnectFromHost();
            return;
        }

        QNetworkRequest request;
        request.setUrl(QUrl::fromEncoded(address));
        queryLines.removeFirst();
        foreach (QByteArray header, queryLines) {
            int sep = header.indexOf(':');
            QByteArray key = header.left(sep).trimmed();
            QByteArray value = header.mid(sep + 1).trimmed();
            if (!key.isEmpty())
                request.setRawHeader(key, value);
        }

        if (socket->localPort() == cachingProxy.serverPort()) {
            QNetworkReply *reply = networkManager.get(request);
            reply->setParent(socket);
            connect(reply, SIGNAL(finished()), this, SLOT(proxyData()));
        } else {
            QByteArray header, data;
            loadFromCache(request.url(), header, data);
            if (header.isEmpty()) {
                header = "HTTP/1.0 404 Not Found\n\n";
                qWarning() << "No cache for" << request.url();
            }

            socket->write(header);
            socket->write(data);
            socket->flush();
            socket->disconnectFromHost();
        }
    }

    void proxyData() {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

        QByteArray data = reply->readAll();
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString phrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        reply->deleteLater();

        QByteArray header;
        header += "HTTP/1.0 " + QString::number(code) + " " + phrase + "\n";
        foreach (QByteArray key, reply->rawHeaderList())
            if (key != "Transfer-Encoding" && key != "Connection" && key != "Keep-Alive")
                header.append(key).append(": ").append(reply->rawHeader(key)).append('\n');
        header += '\n';

        QTcpSocket *socket = qobject_cast<QTcpSocket*>(reply->parent());
        socket->write(header);
        socket->write(data);
        socket->flush();
        socket->disconnectFromHost();

        saveToCache(reply->request().url(), header, data);
    }

private:

    void saveToCache(const QUrl &url, const QByteArray &header, const QByteArray &data)
    {
        QSqlQuery query(database);
        query.prepare("INSERT INTO responses(url, header, data) VALUES(:url, :header, :data)");
        query.bindValue(":url", url.toEncoded());
        query.bindValue(":header", header);
        query.bindValue(":data", data);
        if (!query.exec())
            if (query.lastError().number() == 19) {
                query.prepare("UPDATE responses SET header = :header, data = :data WHERE url = :url");
                query.bindValue(":url", url.toEncoded());
                query.bindValue(":header", header);
                query.bindValue(":data", data);
                query.exec();
            }

        if (query.lastError().type() != QSqlError::NoError)
            qWarning() << "Database error when saving" << url << ":" << query.lastError();
    }

    void loadFromCache(const QUrl &url, QByteArray &header, QByteArray &data)
    {
        QSqlQuery query(database);
        query.prepare("SELECT header, data FROM responses WHERE url = :url");
        query.bindValue(":url", url.toEncoded());
        if (query.exec()) {
            query.next();
            if (query.isValid()) {
                header = query.value(0).toByteArray();
                data = query.value(1).toByteArray();
            }
        }

        if (query.lastError().type() != QSqlError::NoError)
            qWarning() << "Database error when retrieving" << url << ":" << query.lastError();
    }

};

#include "netspiegel.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    NetSpiegel netSpiegel;
    return app.exec();
}
