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
#include <QtWebKit>

#if QT_VERSION < 0x0040600
#error You need Qt 4.6 or newer
#endif

static int distance(const QPoint &p1, const QPoint &p2)
{
    QPoint delta = p1 - p2;
    return delta.x() * delta.x() + delta.y() * delta.y();
}

static int distance(const QPoint &pos, const QRect &rect)
{
    if (rect.contains(pos))
        return 0;
    int d1 = distance(pos, rect.topLeft());
    int d2 = distance(pos, rect.topRight());
    int d3 = distance(pos, rect.bottomLeft());
    int d4 = distance(pos, rect.bottomRight());
    return qMin(qMin(d1, d2), qMin(d3, d4));
}

class ProbeLink: public QWebView
{
private:

    QUrl closestLink(const QPoint &pos) {
        QWebFrame *frame = page()->mainFrame();
        if (!QRect(QPoint(0, 0), frame->contentsSize()).contains(pos))
            return QUrl();
        QWebElement closestElement;
        QUrl closestUrl;
        int closestDistance = 64 * 64;
        foreach (QWebElement element, frame->findAllElements("a")) {
            int dd = distance(pos, element.geometry());
            QUrl url = element.attribute("href");
            url = url.isRelative() ? frame->baseUrl().resolved(url) : url;
            if (url.isValid() && dd < closestDistance) {
                closestDistance = dd;
                closestElement = element;
                closestUrl = url;
            }
        }
        if (closestUrl.isValid() && !closestElement.isNull())
            closestElement.setStyleProperty("background-color", "yellow");
        return closestUrl;
    }

    void mouseReleaseEvent(QMouseEvent *event) {
        QWebFrame *frame = page()->mainFrame();
        QUrl url = closestLink(event->pos() + frame->scrollPosition());
        if (url.isValid()) {
            load(url);
            event->accept();
        } else {
            QWebView::mouseReleaseEvent(event);
        }
    }

};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ProbeLink view;
    QObject::connect(&view, SIGNAL(titleChanged(QString)),
                     &view, SLOT(setWindowTitle(QString)));
    view.load(QUrl("http://en.m.wikipedia.org/"));
    view.resize(320, 480);
    view.show();

    return app.exec();
}
