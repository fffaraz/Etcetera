/*
    This file is part of the Ofi Labs X2 project.

    Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>

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
#include <QtOpenGL>
#include <QtWebKit>

#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
#error You need Qt 4.7 or newer
#endif

class WebView: public QGraphicsView
{
public:
    WebView(QGraphicsScene *scene);
    void toggle();
protected:
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<QGraphicsWebView> content;
    QTime ticker;
    int frameCount;
};

WebView::WebView(QGraphicsScene *scene)
    : QGraphicsView(scene)
    , content(new QGraphicsWebView)
    , frameCount(0)
{
    setFrameStyle(QFrame::NoFrame);
    scene->addItem(content.data());

    content->resize(530, 800);
    content->page()->setViewportSize(QSize(530, 800));

    QWebFrame *frame = content->page()->mainFrame();
    frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    frame->page()->settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, false);

    resize(530, 750);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewport(new QGLWidget(this));

    frameCount = 0;
    ticker.start();
    content->setUrl(QUrl("qrc:/index.html"));
}

void WebView::toggle()
{
    QWebSettings *settings = content->page()->settings();
    bool composited = settings->testAttribute(QWebSettings::AcceleratedCompositingEnabled);
    settings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, !composited);
    frameCount = 0;
    ticker.start();
    setWindowTitle("Wait...");
    content->setUrl(QUrl("qrc:/index.html"));
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    toggle();
    event->accept();
}

class BorderEffect: public QGraphicsEffect
{
public:
    BorderEffect(QObject *parent = 0)
        : QGraphicsEffect(parent)
    {
    }
protected:
    void draw(QPainter *painter)
    {
        drawSource(painter);
        if (sourceBoundingRect().topLeft() == QPoint(0, 0)) {
            painter->save();
            painter->setPen(Qt::yellow);
            painter->drawRect(sourceBoundingRect().adjusted(1, 1, -1, -1));
            painter->restore();
        }
    }
};

// recursively apply the effect to the leaf item
void applyEffect(QGraphicsItem *item)
{
    foreach (QGraphicsItem *child, item->childItems()) {
        if (child->childItems().count()) {
            applyEffect(child);
        } else {
            child->setGraphicsEffect(new BorderEffect);
        }
    }
}

void WebView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        toggle();
        event->accept();
    }
    if (event->key() == Qt::Key_D) {
        applyEffect(content.data());
        event->accept();
    }
}

void WebView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

    frameCount++;
    if (frameCount >= 80) {
        int elapsed = ticker.elapsed();
        ticker.start();
        int fps = frameCount * 1000 / (1 + elapsed);
        bool composited = content->page()->settings()->testAttribute(QWebSettings::AcceleratedCompositingEnabled);
        QString mode = composited ? "Composited" : "Immediate";
        setWindowTitle(QString("Mode: %1      %2 frames/second").arg(mode).arg(fps));
        frameCount = 0;
    }
}

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(composition);

    QApplication::setGraphicsSystem("raster");
    QApplication app(argc, argv);

    QGraphicsScene scene;
    WebView view(&scene);
    view.setWindowTitle("Press Spacebar to switch mode");
    view.show();

    return app.exec();
}
