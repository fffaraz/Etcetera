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

#include <math.h>

#ifdef M_PI
#define M_PI 3.14159265358979323846
#endif

static QPolygonF subdivide(const QPolygonF &p, qreal delta = 8)
{
    QPolygonF poly;
    poly << p.at(0);
    for (int i = 0; i < p.count() - 1; ++i) {
        QPointF d = p.at(i + 1) - p.at(i);
        qreal len = sqrt(d.x() * d.x() + d.y() * d.y());
        poly << p.at(i);
        int split = len / delta;
        for (int c = 1; c < split; ++c)
            poly << p.at(i) + static_cast<qreal>(c) * d / split;
    }
    return poly;
}

static QPolygonF createCircle(qreal radius, int points)
{
    QPolygonF poly;
    for (int i = 0; i < points; ++i) {
        qreal a = i * M_PI * 2 / points;
        poly << QPointF(cos(a), sin(a)) * radius;
    }
    return poly;
}

static QPolygonF createSlab(qreal thickness, qreal length, qreal angle, int points)
{
    QPolygonF poly;

    int cx = points / 2 * thickness / (thickness + length);
    int cy = points / 2 * length / (thickness + length);

    qreal xo = thickness / 2;
    qreal yo = length;

    for (int x = 0; x < cx; ++x)
        poly << QPointF(-xo + x * thickness / cx, length);
    for (int y = 0; y < cy; ++y)
        poly << QPointF(xo, yo - y * length / cy);
    for (int x = 0; x < cx; ++x)
        poly << QPointF(xo - x * thickness / cx, 0);
    for (int y = 0; y < cy; ++y)
        poly << QPointF(-xo, y * length / cy);

    while (poly.count() < points)
        poly << QPointF(-xo, yo);

    poly = QTransform().rotateRadians(M_PI + angle).map(poly);

    return poly;
}

static QPolygonF sort(const QPolygonF &p)
{
    QPolygonF poly;
    int index = 0;
    for (int c = 0; c < p.count(); ++c)
        if (p.at(c).y() > p.at(index).y())
            index = c;
    for (int c = 0; c < p.count(); ++c)
        poly << p.at((index + c) % p.count());
    return poly;
}

template <class T>
static T tween(const T &a, const T &b, qreal t)
{
    return a + t * (b - a);
}

static QPolygonF tween(const QPolygonF &poly1, const QPolygonF &poly2, qreal t)
{
    if (qFuzzyCompare(t, 1 + t))
        return poly1;
    Q_ASSERT(poly1.count() == poly2.count());
    QPolygonF poly;
    poly.reserve(poly1.count());
    for (int i = 0; i < poly1.count(); ++i)
        poly << tween(poly1.at(i), poly2.at(i), t);
    return poly;
}

class MorphingClock: public QWidget
{
    Q_OBJECT

private:
    QTimer timer;
    QTimeLine timeLine;
    qreal radius;
    qreal digitWidth;
    QFont font;
    QPolygonF sourcePoly[4];
    QPolygonF targetPoly[4];
    int targetLevel[4];

public:

    MorphingClock(QWidget *parent = 0)
        : QWidget(parent) {

        connect(&timer, SIGNAL(timeout()), SLOT(tick()));
        timer.start(15 * 1000);

        timeLine.setCurveShape(QTimeLine::EaseInOutCurve);
        connect(&timeLine, SIGNAL(valueChanged(qreal)), SLOT(update()));
        timeLine.setDuration(503);
        timeLine.setCurrentTime(0);
        timeLine.setUpdateInterval(25);
        timeLine.setDirection(QTimeLine::Backward);

        targetLevel[0] = 224;
        targetLevel[1] = 64;
        targetLevel[2] = 64;
        targetLevel[3] = 192;
    }

public slots:

    void toggleType() {
        if (timeLine.state() == QTimeLine::Running)
            return;
        timeLine.toggleDirection();
        timeLine.start();
    }

private slots:

    void tick() {
        int hour = QTime::currentTime().hour();
        int minute = QTime::currentTime().minute();
        QChar text[4];
        text[0] = '0' + (hour / 10);
        text[1] = '0' + (hour % 10);
        text[2] = '0' + (minute / 10);
        text[3] = '0' + (minute % 10);

        for (int c = 0; c < 4; ++c) {
            QPainterPath path;
            path.addText(0, 0, font, text[c]);
            QRectF box = path.boundingRect();
            qreal ofs = (c - 2) * digitWidth;
            ofs += (c < 2) ? -digitWidth / 2 : digitWidth / 2;
            path.translate(ofs, box.height() / 2);
            sourcePoly[c] = sort(subdivide(path.toFillPolygon()));
        }

        qreal angle1 = hour * M_PI / 6 + minute * M_PI / 360;
        qreal angle2 = minute * M_PI / 30;

        targetPoly[0] = createCircle(radius, sourcePoly[0].count());
        targetPoly[1] = createSlab(radius / 6, 0.5 * radius, angle1, sourcePoly[1].count());
        targetPoly[2] = createSlab(radius / 6, 0.9 * radius, angle2, sourcePoly[2].count());
        targetPoly[3] = createCircle(0.8 * radius, sourcePoly[3].count());

        for (int c = 0; c < 4; ++c)
            targetPoly[c] = sort(targetPoly[c]);

        update();
    }

protected:

    void resizeEvent(QResizeEvent *event) {
        Q_UNUSED(event);
        radius = qMin(height(), width()) * 0.45;
        font = QFont("Helvetica", 72);
        qreal fw = QFontMetrics(font).width("4");
        font.setPointSize(72 * (2 * radius / 4.5) / fw);
        digitWidth = QFontMetrics(font).width("4");
        tick();
    }

    void paintEvent(QPaintEvent *event) {
        Q_UNUSED(event);
        QPainter p(this);
        p.fillRect(rect(), Qt::white);
        p.translate(rect().center());
        qreal frame = timeLine.currentValue();
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBrush(Qt::lightGray);
        p.setPen(Qt::NoPen);
        for (int c = 0; c < 4; ++c) {
            static int order[] = { 0, 3, 1, 2 };
            int i = order[c];
            int level = tween(128, targetLevel[i], frame);
            p.setBrush(QColor(level, level, level));
            p.drawPolygon(tween(sourcePoly[i], targetPoly[i], frame));
        }
        p.setOpacity(1 - frame);
        p.drawEllipse(QPointF(0, radius / 10), radius / 20, radius / 20);
        p.drawEllipse(QPointF(0, -radius / 10), radius / 20, radius / 20);
        p.setOpacity(frame);
        p.drawEllipse(QPointF(0, 0), radius / 5, radius / 5);
    }

    void mousePressEvent(QMouseEvent *event) {
        toggleType();
        event->accept();
    }

    void keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Select) {
            toggleType();
            event->accept();
        }
    }
};

#include "morphingclock.moc"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MorphingClock w;
    w.setWindowTitle("Morphing Clock");
    w.resize(500, 500);
    w.show();

    return app.exec();
}
