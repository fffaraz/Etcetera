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
    m_values[1] = -0.2;
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

class Ball
{
public:
    int radius;
    QColor color;
    QPointF position;
    QPointF velocity;
    QSizeF boundary;

private:

    Accelerometer m_accelerometer;

    void bounce(int x, int y) {
        velocity = QPointF(velocity.x() * x, velocity.y() * y);
        velocity *= 0.78;
    }

public:

    Ball() {
        m_accelerometer.start();
    }

    ~Ball() {
        m_accelerometer.stop();
    }

    void update() {

        // acceleration
        qreal xa, ya, za;
        m_accelerometer.getAcceleration(xa, ya, za);

        // velocity
        QPoint dv(-xa * 20, -ya * 20);
        dv *= (1 + za);
        velocity += dv;

        // position
        qreal bx = position.x() + velocity.x();
        qreal by = position.y() + velocity.y();
        if (bx - radius < 0) {
            bx = radius;
            bounce(-1, 1);
        }
        if (by - radius < 0) {
            by = radius;
            bounce(1, -1);
        }
        if (bx + radius > boundary.width()) {
            bx = boundary.width() - radius;
            bounce(-1, 1);
        }
        if (by + radius > boundary.height()) {
            by = boundary.height() - radius;
            bounce(1, -1);
        }
        position = QPointF(bx, by);
    }

    QRect boundingRect() const {
        qreal x = position.x() - radius;
        qreal y = position.y() - radius;
        return QRectF(x, y, radius * 2, radius * 2).toAlignedRect();
    }

    void draw(QPainter *p) {
        bool slow = velocity.manhattanLength() < 15;
        p->setRenderHint(QPainter::Antialiasing, slow);
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawEllipse(position, radius - 1, radius - 1);
    }
};

class BouncingBall: public QWidget
{
    Q_OBJECT

public:

    BouncingBall(QWidget *parent = 0): QWidget(parent) {
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        QTimer::singleShot(200, this, SLOT(start()));
        setWindowTitle("Bouncing Ball");
    }

private slots:

    void start() {
        m_ball.radius = 25;
        m_ball.color = palette().color(QPalette::Highlight);
        m_ball.position = QPointF(width() / 2, height() / 2);
        m_ball.velocity = QPointF(35, 10);
        m_ball.boundary = this->size();
        m_timer.start(30, this);
        update();
    }

protected:

    void timerEvent(QTimerEvent*) {
        update(m_ball.boundingRect());
        m_ball.update();
        update(m_ball.boundingRect());
    }

    void mousePressEvent(QMouseEvent *event) {
        m_ball.position = event->pos();
        m_ball.velocity = QPointF(0, 0);
        m_timer.stop();
        update();
    }

    void mouseReleaseEvent(QMouseEvent*) {
        m_timer.start(30, this);
    }

    void mouseMoveEvent(QMouseEvent *event) {
        if (!m_timer.isActive()) {
            m_ball.position = event->pos();
            update();
        }
    }

    void paintEvent(QPaintEvent *event) {
        QPainter p(this);
        p.fillRect(event->rect(), palette().color(QPalette::Window));
        m_ball.draw(&p);
    }

private:
    Ball m_ball;
    QBasicTimer m_timer;
};

#include "bouncingball.moc"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    BouncingBall view;
    view.resize(800, 480);
    view.show();

    return app.exec();
}
