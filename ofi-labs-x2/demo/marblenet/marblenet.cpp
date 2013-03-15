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

#include <QtGui>
#include <QtNetwork>

#ifdef Q_OS_LINUX
#include <QtDBus>
#endif

#include "chipmunk.h"

#ifdef Q_WS_MAEMO_5
#define LANDSCAPE
#endif

class Accelerometer: public QObject
{
    Q_OBJECT

public:
    Accelerometer(QObject *parent = 0);
    ~Accelerometer();
    void start();
    void stop();
    void getAcceleration(qreal &x, qreal &y, qreal &z);

private slots:
    void internalRun();

private:
    QThread m_thread;
    QMutex m_mutex;
    bool m_terminate;
    qreal m_values[3];
};

Accelerometer::Accelerometer(QObject *parent)
    : QObject(parent)
    , m_terminate(false)
{
    m_values[0] = 0;
    m_values[1] = -1;
    m_values[2] = 0;
    moveToThread(&m_thread);
    m_thread.start();
}

Accelerometer::~Accelerometer()
{
    stop();
}

void Accelerometer::start()
{
    QMetaObject::invokeMethod(this, "internalRun");
}

void Accelerometer::stop()
{
    m_mutex.lock();
    m_terminate = true;
    m_mutex.unlock();
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait();
    }
}

void Accelerometer::internalRun()
{
#ifdef Q_WS_MAEMO_5
    QDBusConnection connection(QDBusConnection::systemBus());
    if (connection.isConnected()) {
        bool terminate = false;
        while (!terminate) {
            QDBusInterface interface("com.nokia.mce", "/com/nokia/icd", QString(), connection);
            QDBusPendingReply<QString, QString, QString, int, int, int> reply;
            reply = interface.asyncCall("get_device_orientation");
            reply.waitForFinished();
            m_mutex.lock();
            m_values[0] = static_cast<qreal>(reply.argumentAt<3>()) / 1000;
            m_values[1] = static_cast<qreal>(reply.argumentAt<4>()) / 1000;
            m_values[2] = static_cast<qreal>(reply.argumentAt<5>()) / 1000;
            terminate = m_terminate;
            m_mutex.unlock();
        }
    }
#endif
}

void Accelerometer::getAcceleration(qreal &x, qreal &y, qreal &z)
{
    m_mutex.lock();
    x = m_values[0];
    y = m_values[1];
    z = m_values[2];
    m_mutex.unlock();
}


QByteArray createDatagram(qint64 pid, int port)
{
    QByteArray datagram;
    datagram.append("marblenet");
    datagram.append('/');
    datagram.append(QString::number(pid));
    datagram.append('/');
    datagram.append(QString::number(port));
    return datagram;
}

bool decodeDatagram(const QByteArray &datagram, qint64 &pid, int &port)
{
    QList<QByteArray> list = datagram.split('/');
    if (list.size() != 3)
        return false;

    QString magicId = list.at(0);
    if (magicId != "marblenet")
        return false;
    pid = list.at(1).toInt();
    port = list.at(2).toInt();
    return true;
}

class Node: public QObject
{
    Q_OBJECT

private:
    QTcpServer tcpServer;
    QHostAddress serverAddress;
    int serverPort;
    QUdpSocket broadcastSocket;
    QTimer broadcastTimer;
    QHostAddress peerAddress;
    int peerPort;

public:

    Node(QObject *parent = 0): QObject(parent) {
        init();
    }

    QString peerInfo() const {
        return QString("%1:%2").arg(peerAddress.toString()).arg(peerPort);
    }

    bool send(const QByteArray &data) {
        QTcpSocket socket;
        socket.connectToHost(peerAddress, peerPort);
        if (!socket.waitForConnected(1000))
            return false;
        socket.write(data);
        socket.flush();
        socket.waitForBytesWritten();
        return socket.error() == QTcpSocket::UnknownSocketError;
    }

    void setPeer(QHostAddress addr, int port) {
        peerAddress = addr;
        peerPort = port;
        broadcastSocket.close();
        broadcastTimer.stop();
    }

signals:
    void peerAvailable();
    void receivedData(const QByteArray &data);

private slots:

    void init() {
        tcpServer.listen();
        connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(handleConnection()));
        foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
            quint32 ipv4addr = address.toIPv4Address();
            if (ipv4addr && QHostAddress(ipv4addr) != QHostAddress::LocalHost)
                serverAddress = QHostAddress(ipv4addr);
        }
        serverPort = tcpServer.serverPort();

        QUdpSocket::BindMode mode = QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint;
        broadcastSocket.bind(QHostAddress::Any, 45000, mode);
        connect(&broadcastSocket, SIGNAL(readyRead()), SLOT(readBroadcast()));

        broadcastTimer.setInterval(5 * 1000);
        connect(&broadcastTimer, SIGNAL(timeout()), SLOT(broadcast()));
        broadcastTimer.start();
    }

    void broadcast() {
        QByteArray datagram;
        datagram = createDatagram(QApplication::applicationPid(), serverPort);

        QList<QHostAddress> broadcastAddresses;
        foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
            foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
                QHostAddress broadcastAddress = entry.broadcast();
                if (broadcastAddress != QHostAddress::Null &&
                    entry.ip() != QHostAddress::LocalHost &&
                    !broadcastAddress.toString().startsWith("127."))
                    broadcastAddresses << broadcastAddress;
            }
        }

        foreach (QHostAddress address, broadcastAddresses)
            broadcastSocket.writeDatagram(datagram, address, 45000);
    }

    void readBroadcast() {
        bool peerAvail = false;
        while (!peerAvail && broadcastSocket.hasPendingDatagrams()) {
            QHostAddress senderIp;
            quint16 senderPort;
            QByteArray datagram;
            qint64 pid;
            int port;
            datagram.resize(broadcastSocket.pendingDatagramSize());
            if (broadcastSocket.readDatagram(datagram.data(), datagram.size(),
                                             &senderIp, &senderPort) == -1)
                continue;
            if (!decodeDatagram(datagram, pid, port))
                continue;
            if (pid == QApplication::applicationPid() && port == serverPort)
                continue;
            peerAddress = senderIp;
            peerPort = port;
            peerAvail = true;
        }
        if (peerAvail)
            emit peerAvailable();
    }

    void handleConnection() {
        QTcpSocket *socket = tcpServer.nextPendingConnection();
        if (!socket)
            return;
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(handleIncomingData()));
    }

    void handleIncomingData() {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket)
            return;
        socket->waitForReadyRead(1000);
        QByteArray data = socket->readAll();
        emit receivedData(data);
    }
};

const qreal fps = 50;

class MarbleNet: public QWidget
{
    Q_OBJECT
public:
    MarbleNet(QWidget *parent = 0): QWidget(parent) {
        m_accelerometer.start();
        m_node = new Node(this);
        connect(m_node, SIGNAL(peerAvailable()), this, SLOT(updatePeerStatus()));
        connect(m_node, SIGNAL(receivedData(QByteArray)), this, SLOT(receiveBall(QByteArray)));

        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_NoSystemBackground, true);

#ifdef LANDSCAPE
        resize(800, 480);
#else
        resize(480, 600);
#endif

        setWindowTitle("Box of Marbles");
        QTimer::singleShot(200, this, SLOT(start()));
    }

    ~MarbleNet() {
        cpBodyFree(m_container);
        cpSpaceFreeChildren(m_space);
        cpSpaceFree(m_space);
    }

private slots:

    void updatePeerStatus() {
        setWindowTitle(QString("Box of Marbles [%1]").arg(m_node->peerInfo()));
    }

    void start() {
        QStringList args = QApplication::arguments();
        if (QApplication::arguments().count() == 3)
            m_node->setPeer(QHostAddress(args[1]), args[2].toInt());

        m_timer.start(1000 / fps, this);
        setFixedSize(size());
        qreal hw = width() / 2;
        qreal hh = height() / 2;
        m_transform.translate(hw, hh);
        m_transform.scale(1, -1);

        cpInitChipmunk();
        m_container = cpBodyNew(INFINITY, INFINITY);

        m_space = cpSpaceNew();
        m_space->iterations = 15;

#ifdef LANDSCAPE
        addSegment(hw, -hh,  hw,  hh);
        addSegment(-hw, -hh,  hw, -hh);
        addSegment(-hw,  hh,  hw,  hh);
#else
        addSegment(-hw, -hh, -hw,  hh);
        addSegment(hw, -hh,  hw,  hh);
        addSegment(-hw, -hh,  hw, -hh);
#endif
        addCircle(0, 0);
    }

    void receiveBall(const QByteArray &data) {
        QList<QByteArray> list = data.split(':');
        if (list.size() != 6)
            return;
        if (list.at(0) != "ball")
            return;
        qreal cx = list.at(1).toDouble();
        qreal cy = list.at(2).toDouble();
        qreal vx = list.at(3).toDouble();
        qreal vy = list.at(4).toDouble();
        int hue = list.at(5).toInt();
        QColor color = QColor::fromHsv(hue, 255, 255);
#ifdef LANDSCAPE
        addCircle(-width() / 2 + cy, -cx, vy, -vx, color);
#else
        addCircle(-cx, height() / 2 - cy, -vx, -vy, color);
#endif
    }

private:

    void addCircle(qreal cx, qreal cy, qreal vx = 0, qreal vy = 0, QColor c = QColor()) {
        cpBody *body = cpBodyNew(100, 3000);
        body->p = cpv(cx, cy);
        body->v = cpv(vx, vy);
        cpSpaceAddBody(m_space, body);

        cpShape *shape = cpCircleShapeNew(body, 30, cpvzero);
        shape->e = 0.78;
        shape->u = 0.9;
        cpSpaceAddShape(m_space, shape);
        m_circles += (cpCircleShape*)(shape);

        if (!c.isValid())
            c = QColor::fromHsv(qrand() % 360, 255, 255);
        m_colors += c;
    }

    void addSegment(qreal x1, qreal y1, qreal x2, qreal y2) {
        cpShape *shape;
        shape = cpSpaceAddStaticShape(m_space,
                                      cpSegmentShapeNew(m_container,
                                                        cpv(x1, y1),
                                                        cpv(x2, y2), 0));
        shape->e = 1;
        shape->u = 1;
    }

protected:
    void timerEvent(QTimerEvent *) {
        qreal xa, ya, za;
        m_accelerometer.getAcceleration(xa, ya, za);
        xa *= (1 + za);
        ya *= (1 + za);
        m_space->gravity = cpv(-xa * 2000, ya * 2000);
        cpSpaceStep(m_space, 1.0 / fps);
        update();

        QList<cpCircleShape*> teleportList;
        QList<QColor> teleportColor;
        for (int i = 0; i < m_circles.count(); ++i) {
            cpCircleShape *circle = m_circles.at(i);
#ifdef LANDSCAPE
            if (circle->tc.x < -width() / 2) {
                teleportList += circle;
                teleportColor += m_colors.at(i);
                m_colors.removeAt(i);
            }
#else
            if (circle->tc.y > height() / 2) {
                teleportList += circle;
                teleportColor += m_colors.at(i);
                m_colors.removeAt(i);
            }
#endif
        }
        if (teleportList.count())
            for (int i = 0; i < teleportList.count(); ++i) {
                cpCircleShape *circle = teleportList.at(i);
                m_circles.removeAll(circle);
                cpSpaceRemoveShape(m_space, (cpShape*)(circle));
                QByteArray data;
                data.append("ball:");
#ifdef LANDSCAPE
                data.append(QString::number(circle->tc.y));
                data.append(':');
                data.append(QString::number(-width() / 2 - circle->tc.x));
                data.append(':');
                data.append(QString::number(-circle->shape.body->v.y));
                data.append(':');
                data.append(QString::number(-circle->shape.body->v.x));
#else
                data.append(QString::number(circle->tc.x));
                data.append(':');
                data.append(QString::number(circle->tc.y - height() / 2));
                data.append(':');
                data.append(QString::number(circle->shape.body->v.x));
                data.append(':');
                data.append(QString::number(circle->shape.body->v.y));
#endif
                data.append(':');
                data.append(QString::number(teleportColor.at(i).toHsv().hue()));
                m_node->send(data);
            }
    }

    void paintEvent(QPaintEvent *event) {
        QPainter p(this);
        p.fillRect(event->rect(), Qt::black);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setTransform(m_transform);
        p.setPen(Qt::NoPen);
        for (int i = 0; i < m_circles.count(); ++i) {
            const cpCircleShape *circle = m_circles.at(i);
            p.setBrush(m_colors[i]);
            p.drawEllipse(QPointF(circle->tc.x, circle->tc.y), circle->r, circle->r);
        }
    }

    void mousePressEvent(QMouseEvent *event) {
        QPointF pos = m_transform.inverted().map(event->pos());
        addCircle(pos.x(), pos.y());
    }

private:
    Accelerometer m_accelerometer;
    Node *m_node;
    QBasicTimer m_timer;
    QTransform m_transform;
    cpBody *m_container;
    cpSpace *m_space;
    QList<cpCircleShape*> m_circles;
    QList<QColor> m_colors;
};

#include "marblenet.moc"

int main(int argc, char **argv)
{
#ifdef Q_WS_MAEMO_5
    QApplication::setGraphicsSystem("raster");
#endif

    QApplication app(argc, argv);
    qsrand(QTime::currentTime().msec());

    MarbleNet box;
    box.show();

    return app.exec();
}
