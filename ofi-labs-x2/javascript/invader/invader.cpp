/*
    This file is part of the Ofi Labs X2 project.

    Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

    Using "RGB Invaders" JavaScript code and sprites from Egor Balishev
    (see http://10k.aneventapart.com/Entry/392)

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
#include <QtScript>

static QString readFile(const QString &fileName)
{
    QFile file;
    file.setFileName(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCritical() << "Can't open" << fileName;
        return QString();
    }
    QString content = file.readAll();
    file.close();
    return content;
}

static QScriptValue setTimeout(QScriptContext *context, QScriptEngine *engine);
static QScriptValue setInterval(QScriptContext *context, QScriptEngine *engine);

class Invader : public QWidget
{
    Q_OBJECT

public:
    Invader(QWidget *parent = 0);

public slots:
    void invokeFunction();
    QList<int> preloadImage(const QString &src);
    void drawImage(const QString &source, double x, double y);
    QList<int> getPixels(int x, int y, int w, int h);
    void putPixels(int x, int y, int w, int h, const QList<int> &pixels);
    void fillRect(qreal x, qreal y, qreal w, qreal h, const QString &color, qreal alpha);
    void beginPath();
    void arc(qreal x, qreal y, qreal r);
    void fill(const QString &color, const QString &globalAlpha);
    void stroke(const QString &color, const QString &globalAlpha);

public:
    QScriptValue pendingFunction;
    QScriptValue intervalFunction;

protected:
    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QScriptEngine *engine;
    QImage buffer;
    QHash<QString, QImage> images;
    QPainterPath path;
};

Invader::Invader(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    engine = new QScriptEngine(this);
    engine->globalObject().setProperty("setTimeout", engine->newFunction(setTimeout, 2));
    engine->globalObject().setProperty("setInterval", engine->newFunction(setInterval, 2));
    engine->globalObject().setProperty("invader", engine->newQObject(this));

    engine->evaluate(readFile(":/magic.js"));
    engine->evaluate(readFile(":/invader.js"));

    startTimer(15);
}

void Invader::invokeFunction()
{
   pendingFunction.call();
}

QList<int> Invader::preloadImage(const QString &source)
{
    QList<int> size;
    size += 0;
    size += 0;

    QImage img;
    QString fname = ":" + source;
    img.load(fname);
    if (img.isNull())
        return size;

    images[source] = img;
    size[0] = img.width();
    size[1] = img.height();
    return size;
}

void Invader::drawImage(const QString &source, double x, double y)
{
    if (!images.contains(source))
        preloadImage(source);
    QPainter p(&buffer);
    p.drawImage(x, y, images[source]);
    p.end();
}

QList<int> Invader::getPixels(int x, int y, int w, int h)
{
    QList<int> pixels;

    int x1 = qBound(0, x, buffer.width() - 1);
    int x2 = qBound(0, x + w, buffer.width() - 1);
    int y1 = qBound(0, y, buffer.height() - 1);
    int y2 = qBound(0, y + h, buffer.height() - 1);

    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            QRgb pixel = buffer.pixel(x ,y);
            pixels += qRed(pixel);
            pixels += qGreen(pixel);
            pixels += qBlue(pixel);
            pixels += qAlpha(pixel);
        }
    }
    return pixels;
}

void Invader::putPixels(int x, int y, int w, int h, const QList<int> &pixels)
{
    Q_UNUSED(h);
    for (int i = 0; i < pixels.length(); i += 4) {
        int r = pixels[i];
        int g = pixels[i + 1];
        int b = pixels[i + 2];
        int a = pixels[i + 3];
        int posx = x + (i / 4) % w;
        int posy = y + (i / 4) / w;
        if (posx < 0 || posx >= buffer.width())
            continue;
        if (posy < 0 || posy >= buffer.height())
            continue;
        buffer.setPixel(posx, posy, qRgba(r, g, b, a));
    }
    update();
}

void Invader::fillRect(qreal x, qreal y, qreal w, qreal h, const QString &color, qreal alpha)
{
    QPainter p(&buffer);
    p.setOpacity(alpha);
    p.fillRect(x, y, w, h, QColor(color));
    p.end();
    update();
}

void Invader::beginPath()
{
    path = QPainterPath();
}

void Invader::arc(qreal x, qreal y, qreal r)
{
    path.addEllipse(QPointF(x, y), r, r);
}

void Invader::fill(const QString &color, const QString &globalAlpha)
{
    QPainter p(&buffer);
    if (!globalAlpha.isEmpty() && globalAlpha != "defined")
        p.setOpacity(globalAlpha.toFloat());
    p.setBrush(QColor(color));
    p.setPen(Qt::NoPen);
    p.drawPath(path);
    p.end();
    update();
}

void Invader::stroke(const QString &color, const QString &globalAlpha)
{
    QPainter p(&buffer);
    p.setPen(QPen(QColor(color), 1));
    if (!globalAlpha.isEmpty() && globalAlpha != "defined")
        p.setOpacity(globalAlpha.toFloat());
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    p.end();
    update();
}

void Invader::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    if (intervalFunction.isValid())
        intervalFunction.call();
}

void Invader::paintEvent(QPaintEvent *event)
{
    if (buffer.size() != size()) {
        buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        buffer.fill(qRgb(0, 0, 0));
        return;
    }
    QPainter p(this);
    p.drawImage(0, 0, buffer);
    p.end();
}

void Invader::keyPressEvent(QKeyEvent *event)
{
    int code = (event->key() == Qt::Key_Space) ? 32 :
               (event->key() == Qt::Key_Left) ? 37 :
               (event->key() == Qt::Key_Right) ? 39 : 0;
    if (code) {
        QString str = QString("window.onkeydown( { type: 'keydown', keyCode: %1 } );").arg(code);
        engine->evaluate(str);
    } else {
        QWidget::keyPressEvent(event);
    }
}

void Invader::keyReleaseEvent(QKeyEvent *event)
{
    int code = (event->key() == Qt::Key_Space) ? 32 :
               (event->key() == Qt::Key_Left) ? 37 :
               (event->key() == Qt::Key_Right) ? 39 : 0;
    if (code) {
        QString str = QString("window.onkeyup( { type: 'keyup', keyCode: %1 } );").arg(code);
        engine->evaluate(str);
    } else {
        QWidget::keyReleaseEvent(event);
   }
}

static QScriptValue setTimeout(QScriptContext *context, QScriptEngine *engine)
{
    Invader *invader = static_cast<Invader*>(engine->parent());

    QScriptValue function = context->argument(0);
    int delay = context->argument(1).toInt32();
    invader->pendingFunction = function;
    QTimer::singleShot(delay, invader, SLOT(invokeFunction()));

    return QScriptValue();
}

static QScriptValue setInterval(QScriptContext *context, QScriptEngine *engine)
{
    Invader *invader = static_cast<Invader*>(engine->parent());

    QScriptValue function = context->argument(0);
    invader->intervalFunction = function;

    return QScriptValue();
}

#include "invader.moc"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(invader);

    QApplication::setGraphicsSystem("raster");
    QApplication app(argc, argv);

    Invader invader;
    invader.setWindowTitle("Invader");
    invader.setFixedSize(700, 600);
    invader.show();

    return app.exec();
}
