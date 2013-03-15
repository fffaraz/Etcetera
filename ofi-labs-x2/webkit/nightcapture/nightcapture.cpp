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
#include <QtWebKit>

class NightCanvas;

class NightPaintEngine: public QPaintEngine
{
public:
    NightPaintEngine(NightCanvas *canvas);
    virtual ~NightPaintEngine();
    bool begin(QPaintDevice *pdev);
    bool end();
    Type type() const;
    void updateState(const QPaintEngineState &state);
    void drawPath(const QPainterPath &path);
    void drawPoints(const QPointF *points, int pointCount);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
private:
    NightCanvas *canvas;
};

class NightCanvas: public QPaintDevice
{
public:
    NightCanvas(int width, int height);
    ~NightCanvas();
    QPaintEngine *paintEngine() const;
    QSize size() const;
    QImage convertToFormat(QImage::Format format) const;
protected:
    int metric(PaintDeviceMetric metric) const;
private:
    friend class NightPaintEngine;
    NightPaintEngine *m_engine;
    QImage m_image;
    QPainter *m_painter;
};

NightCanvas::NightCanvas(int width, int height)
    : QPaintDevice()
    , m_painter(new QPainter)
{
    m_image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(0);
    m_painter->begin(&m_image);
    m_engine = new NightPaintEngine(this);
}

NightCanvas::~NightCanvas()
{
    m_painter->end();
    delete m_engine;
    delete m_painter;
}

QPaintEngine *NightCanvas::paintEngine() const
{
    return m_engine;
}

QSize NightCanvas::size() const
{
    return m_image.size();
}

int NightCanvas::metric(PaintDeviceMetric metric) const
{
    if (metric == PdmWidth)
        return m_image.width();

    if (metric == PdmHeight)
        return m_image.height();

    if (metric == PdmNumColors)
        return INT_MAX;

    if (metric == PdmDepth)
        return 32;

    if (metric == PdmDpiX || metric == PdmDpiY)
        return 72;

    return 0;
}

QImage NightCanvas::convertToFormat(QImage::Format format) const
{
    return m_image.convertToFormat(format);
}

NightPaintEngine::NightPaintEngine(NightCanvas *c)
    : QPaintEngine(QPaintEngine::AllFeatures)
    , canvas(c)
{
}

NightPaintEngine::~NightPaintEngine()
{
}

bool NightPaintEngine::begin(QPaintDevice *pdev)
{
    if (canvas == static_cast<NightCanvas*>(pdev)) {
        setActive(true);
        return true;
    }
    return false;
}

bool NightPaintEngine::end()
{
    setActive(false);
    return true;
}

QPaintEngine::Type NightPaintEngine::type() const
{
    return static_cast<QPaintEngine::Type>(QPaintEngine::User + 42);
}

void NightPaintEngine::updateState(const QPaintEngineState &state)
{
    QPainter *painter = canvas->m_painter;
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & QPaintEngine::DirtyBackground)
        painter->setBackground(state.backgroundBrush());

    if (flags & QPaintEngine::DirtyBackgroundMode)
        painter->setBackgroundMode(state.backgroundMode());

    if (flags & QPaintEngine::DirtyBrush)
        painter->setBrush(state.brush());

    if (flags & QPaintEngine::DirtyBrushOrigin)
        painter->setBrushOrigin(state.brushOrigin());

    if (flags & QPaintEngine::DirtyClipEnabled)
        painter->setClipping(state.isClipEnabled());

    if (flags & QPaintEngine::DirtyClipPath)
        painter->setClipPath(state.clipPath(), state.clipOperation());

    if (flags & QPaintEngine::DirtyClipRegion)
        painter->setClipRegion(state.clipRegion(), state.clipOperation());

    if (flags & QPaintEngine::DirtyCompositionMode)
        painter->setCompositionMode(state.compositionMode());

    if (flags & QPaintEngine::DirtyFont)
        painter->setFont(state.font());

    if (flags & QPaintEngine::DirtyHints)
        painter->setRenderHints(state.renderHints());

    if (flags & QPaintEngine::DirtyPen)
        painter->setPen(state.pen());

    if (flags & QPaintEngine::DirtyTransform)
        painter->setTransform(state.transform());
}

void NightPaintEngine::drawPath(const QPainterPath &path)
{
    canvas->m_painter->drawPath(path);
}

void NightPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    canvas->m_painter->drawPoints(points, pointCount);
}

void NightPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    canvas->m_painter->drawPolygon(points, pointCount, mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
}

void NightPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    QImage img = pm.toImage();
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_Difference);
    p.fillRect(img.rect(), Qt::white);
    p.end();
    canvas->m_painter->drawImage(r, img, sr);
}

class NightCapture : public QObject
{
    Q_OBJECT

public:
    NightCanvas buffer;
    NightCapture();
    void load(const QUrl &url, const QString &outputFileName);

signals:
    void finished();

private slots:
    void saveResult(bool ok);

private:
    QWebPage m_page;
    QString m_fileName;
};

NightCapture::NightCapture()
    : QObject()
    , buffer(320, 480)
{
    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));
}

void NightCapture::load(const QUrl &url, const QString &outputFileName)
{
    m_fileName = outputFileName;
    m_page.mainFrame()->load(url);
    m_page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    m_page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_page.setViewportSize(buffer.size());
}

void NightCapture::saveResult(bool ok)
{
    if (!ok) {
        qCritical() << "Failed loading " << m_page.mainFrame()->url();
        emit finished();
        return;
    }

    QPainter p(&buffer);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    m_page.setViewportSize(m_page.mainFrame()->contentsSize());
    m_page.mainFrame()->render(&p);
    p.end();

    QImage buf = buffer.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    p.begin(&buf);
    p.setCompositionMode(QPainter::CompositionMode_Difference);
    p.fillRect(buf.rect(), Qt::white);
    p.end();

    buf.save(m_fileName);
    emit finished();
}

#include "nightcapture.moc"

#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (argc != 3) {
        std::cout << "nightcapture url outputfile" << std::endl << std::endl;
        return -1;
    }

    QUrl url = QUrl::fromUserInput(QString::fromLocal8Bit(argv[1]));
    QString fileName = QString::fromLocal8Bit(argv[2]);
    NightCapture capture;
    QObject::connect(&capture, SIGNAL(finished()), QApplication::instance(), SLOT(quit()));
    capture.load(url, fileName);

    return app.exec();
}
