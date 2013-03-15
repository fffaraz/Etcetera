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

#ifdef Q_OS_LINUX
#include <QtDBus>
#endif

#include "chipmunk.h"

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
    m_values[1] = -1.7;
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

const qreal radius = 30;
const qreal fps = 55;

class MarbleBox: public QWidget
{
    Q_OBJECT
public:
    MarbleBox(QWidget *parent = 0): QWidget(parent) {
        setWindowTitle("Box of Marbles");
        QTimer::singleShot(200, this, SLOT(start()));
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        m_accelerometer.start();
    }

    ~MarbleBox() {
        cpBodyFree(m_container);
        cpSpaceFreeChildren(m_space);
        cpSpaceFree(m_space);
    }

private slots:

    void start() {
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

        addSegment(-hw, -hh, -hw,  hh);
        addSegment( hw, -hh,  hw,  hh);
        addSegment(-hw, -hh,  hw, -hh);
        addSegment(-hw,  hh,  hw,  hh);
        addCircle(0, 0, radius);
    }

private:

    void addCircle(qreal cx, qreal cy, qreal radius) {
        cpBody *body = cpBodyNew(100, 3000);
        body->p = cpv(cx, cy);
        cpSpaceAddBody(m_space, body);

        cpShape *shape = cpCircleShapeNew(body, radius, cpvzero);
        shape->e = 0.78;
        shape->u = 0.9;
        cpSpaceAddShape(m_space, shape);
        m_circles += (cpCircleShape*)(shape);
        m_colors += QColor::fromHsv(qrand() % 360, 255, 255);
    }

    void addSegment(qreal x1, qreal y1, qreal x2, qreal y2) {
        cpShape *shape;
        shape = cpSpaceAddStaticShape(m_space, cpSegmentShapeNew(
                m_container,cpv(x1, y1), cpv(x2, y2), 0));
        shape->e = 1;
        shape->u = 1;
    }

protected:
    void timerEvent(QTimerEvent *) {
        qreal xa, ya, za;
        m_accelerometer.getAcceleration(xa, ya, za);
        xa *= (1 + za);
        ya *= (1 + za);
        m_space->gravity = cpv(-xa * 1500, ya * 1500);
        cpSpaceStep(m_space, 1 / fps);
        update();
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
        addCircle(pos.x(), pos.y(), radius);
    }

private:
    Accelerometer m_accelerometer;
    QBasicTimer m_timer;
    QTransform m_transform;
    cpBody *m_container;
    cpSpace *m_space;
    QList<cpCircleShape*> m_circles;
    QList<QColor> m_colors;
};

#include "marblebox.moc"

int main(int argc, char **argv)
{
#ifdef Q_WS_MAEMO_5
    QApplication::setGraphicsSystem("raster");
#endif
    QApplication app(argc, argv);
    qsrand(QTime::currentTime().msec());

    MarbleBox box;
    box.resize(800, 480);
    box.show();

    return app.exec();
}
