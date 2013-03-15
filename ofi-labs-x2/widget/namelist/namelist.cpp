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

#include <kineticmodel.h>

class NameList: public QWidget
{
    Q_OBJECT

public:
    NameList(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void changePosition(qreal pos);

private:
    QStringList names;
    int rowHeight;
    int offset;
    KineticModel verticalKinetic;
    bool pressed;
    int refY;
};

NameList::NameList(QWidget *parent)
    : QWidget(parent)
    , offset(0)
    , pressed(false)
    , refY(0)
{
    QFile file(":/names.txt");
    file.open(QFile::ReadOnly);
    QString content = file.readAll();
    names = content.split('\n');
    file.close();

    rowHeight = QFontMetrics(font()).height() + 5;

    connect(&verticalKinetic, SIGNAL(positionChanged(qreal)),
            this, SLOT(changePosition(qreal)));

    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
}

void NameList::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(event->rect(), Qt::white);
    int start = offset / rowHeight;
    int y = start * rowHeight - offset;
    if (offset <= 0) {
        start = 0;
        y = -offset;
    }
    int end = start + height() / rowHeight + 1;
    end = qMin(end, names.count() - 1);
    for (int i = start; i <= end; ++i, y += rowHeight) {
        p.setBrush(Qt::NoBrush);
        p.setPen(Qt::black);
        p.drawText(2, y, width(), rowHeight, Qt::AlignVCenter, names[i]);
    }
}

void NameList::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        pressed = true;
        refY = event->pos().y();
        verticalKinetic.resetSpeed();
    }
}

void NameList::mouseMoveEvent(QMouseEvent *event)
{
    if (pressed) {
        verticalKinetic.setPosition(offset - event->y() + refY);
        refY = event->y();
    }
}

void NameList::mouseReleaseEvent(QMouseEvent *event)
{
    pressed = false;
    refY = event->y();
    verticalKinetic.release();
}

void NameList::changePosition(qreal pos)
{
    offset = pos;
    update();
}

#include "namelist.moc"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    NameList contactList;
    contactList.setWindowTitle("Names");
    contactList.resize(320, 480);
    contactList.show();
    return app.exec();
}
