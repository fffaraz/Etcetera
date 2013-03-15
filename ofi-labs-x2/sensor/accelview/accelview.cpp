/*
    This file is part of the OfiLabs X2 project.

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
#include <QtDBus>

void getAcceleration(qreal &x, qreal &y, qreal &z)
{
#ifdef Q_WS_MAEMO_5
    QDBusConnection connection(QDBusConnection::systemBus());
    if (connection.isConnected()) {
        QDBusInterface interface("com.nokia.mce", "/com/nokia/icd", QString(), connection);
        QDBusPendingReply<QString, QString, QString, int, int, int> reply;
        reply = interface.asyncCall("get_device_orientation");
        reply.waitForFinished();
        x = static_cast<qreal>(reply.argumentAt<3>()) / 1000;
        y = static_cast<qreal>(reply.argumentAt<4>()) / 1000;
        z = static_cast<qreal>(reply.argumentAt<5>()) / 1000;
    }
#endif
}

#include "ui_display.h"

class AccelView: public QWidget
{
public:
    AccelView(QWidget *parent = 0): QWidget(parent) {
        form.setupUi(this);
        timer.start(100, this);
    }

protected:
    void timerEvent(QTimerEvent*) {
        qreal xa = 0;
        qreal ya = -0.2;
        qreal za = 0;
        getAcceleration(xa, ya, za);
        int x = xa * 1000;
        int y = ya * 1000;
        int z = za * 1000;
        form.xSlider->setValue(x);
        form.ySlider->setValue(y);
        form.zSlider->setValue(z);
        form.xValue->setText(QString::number(x));
        form.yValue->setText(QString::number(y));
        form.zValue->setText(QString::number(z));
    }

private:
    Ui::Form form;
    QBasicTimer timer;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    AccelView w;
    w.show();

    return app.exec();
}
