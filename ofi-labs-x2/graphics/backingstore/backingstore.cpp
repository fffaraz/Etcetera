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
#include <QtSvg>

// gl.h on Windows is OpenGL 1.1. GL_CLAMP_TO_EDGE is missing.
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// Uncomment to follow the slower update and refresh
// #define TILE_DEBUG

const int TileDim = 128;

const qreal MinZoom = 0.2;
const qreal MaxZoom = 20;

#ifdef TILE_DEBUG
const int UpdateDelay = 100;
const int RefreshDelay = 600;
const int ExtraTiles = 1;
#else
const int UpdateDelay = 10;
const int RefreshDelay = 200;
const int ExtraTiles = 2;
#endif

// The default texture holds a typical checkerboard pattern, used
// as "placeholder" for outdated tiles.
static QImage createCheckerboardPattern(int dim)
{
    QImage pattern(dim, dim, QImage::Format_ARGB32);
    QRgb color1 = qRgb(240, 240, 240);
    QRgb color2 = qRgb(255, 255, 255);
#ifdef TILE_DEBUG
    color2 = qRgb(128, 128, 128);
#endif
    for (int y = 0; y < TileDim; ++y)
        for (int x = 0; x < TileDim; ++x)
            pattern.setPixel(x, y, (((x >> 4) + (y >> 4)) & 1) ? color1 : color2);
    return pattern;
}

class TextureBuffer
{
public:
    qreal zoomFactor;

    TextureBuffer();
    TextureBuffer& operator=(const TextureBuffer&);

    bool isEmpty() const;
    void clear();

    GLuint at(int x, int y) const;
    void replace(int x, int y, GLuint texture);
    void remove(int x, int y);

    int width() const;
    int height() const;
    void resize(int w, int h);

    void draw(int x, int y, GLuint substitute = 0) const;
    void setViewModelMatrix(const QPointF &viewOffset, qreal viewZoomFactor) const;
    QRect visibleRange(const QPointF &viewOffset, qreal viewZoomFactor,
                     const QSize &viewSize) const;

private:
    QVector<GLuint> textures;
    QSize bufferSize;
};

TextureBuffer::TextureBuffer()
    : zoomFactor(0.1)
{
}

TextureBuffer& TextureBuffer::operator =(const TextureBuffer &buffer)
{
    textures = buffer.textures;
    zoomFactor = buffer.zoomFactor;
    bufferSize = buffer.bufferSize;
    return *this;
}

bool TextureBuffer::isEmpty() const
{
    return textures.isEmpty();
}

void TextureBuffer::clear()
{
    if (!textures.isEmpty()) {
        glDeleteTextures(textures.count(), textures.constData());
        textures.clear();
    }
    bufferSize = QSize(0, 0);
}

GLuint TextureBuffer::at(int x, int y) const
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.count())
        return 0;
    return textures.at(index);
}

// Note: does not free the existing texture!
void TextureBuffer::replace(int x, int y, GLuint texture)
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.count())
        return;
    textures[index] = texture;
}

void TextureBuffer::remove(int x, int y)
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.count())
        return;
    if (textures.at(index) != 0) {
        glDeleteTextures(1, textures.constData() + index);
        textures[index] = 0;
    }
}

int TextureBuffer::width() const
{
    return bufferSize.width();
}

int TextureBuffer::height() const
{
    return bufferSize.height();
}

void TextureBuffer::resize(int w, int h)
{
    bufferSize = QSize(w, h);
    textures.resize(w * h);
    textures.fill(0);
}

// Each tile is drawn as textured quad.
void TextureBuffer::draw(int x, int y, GLuint substitute) const
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.count())
        return;

    GLuint texture = textures.at(index);
    if (texture == 0)
        texture = substitute;
    if (texture == 0)
        return;

    float tx = x * TileDim;
    float ty = y * TileDim;

    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(tx, ty);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(tx + TileDim, ty);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(tx + TileDim, ty + TileDim);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(tx, ty + TileDim);
    glEnd();
}

void TextureBuffer::setViewModelMatrix(const QPointF &viewOffset, qreal viewZoomFactor) const
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(viewOffset.x(), viewOffset.y(), 0);
    glScalef(viewZoomFactor / zoomFactor, viewZoomFactor / zoomFactor, 1);
}

QRect TextureBuffer::visibleRange(const QPointF &viewOffset,
    qreal viewZoomFactor, const QSize &viewSize) const
{
    qreal dim = TileDim * viewZoomFactor / zoomFactor;

    int tx1 = -viewOffset.x() / dim;
    int tx2 = (viewSize.width() -viewOffset.x()) / dim;
    int ty1 = -viewOffset.y() / dim;
    int ty2 = (viewSize.height() - viewOffset.y()) / dim;

    tx1 = qMax(0, tx1);
    tx2 = qMin(bufferSize.width() - 1, tx2);
    ty1 = qMax(0, ty1);
    ty2 = qMin(bufferSize.height() - 1, ty2);

    return QRect(QPoint(tx1, ty1), QPoint(tx2, ty2));
}


class GLTiger: public QGLWidget
{
public:
    GLTiger(QWidget *parent = 0);

private:
    void scheduleUpdate();
    void scheduleRefresh();
    void updateBackingStore();
    void refreshBackingStore();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void timerEvent(QTimerEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseZoom(qreal zoomFactor, QPoint pos);
    void wheelEvent(QWheelEvent *event);

private:
    QPoint m_mousePressPosition;
    int m_updateTimer;
    int m_refreshTimer;

    QPointF m_viewOffset;
    qreal m_viewZoomFactor;
    QSvgRenderer m_svg;

    GLuint m_defaultTexture;
    TextureBuffer m_mainBuffer;
    TextureBuffer m_secondaryBuffer;
};

GLTiger::GLTiger(QWidget *parent)
    : QGLWidget(parent)
    , m_mousePressPosition(0, 0)
    , m_updateTimer(-1)
    , m_refreshTimer(-1)
    , m_viewOffset(0, 0)
    , m_viewZoomFactor(1)
    , m_defaultTexture(0)
{
    setAttribute(Qt::WA_NoSystemBackground);
}

void GLTiger::scheduleUpdate()
{
    killTimer(m_updateTimer);
    m_updateTimer = startTimer(UpdateDelay);
}

void GLTiger::scheduleRefresh()
{
    killTimer(m_refreshTimer);
    m_refreshTimer = startTimer(RefreshDelay);
}

void GLTiger::refreshBackingStore()
{
    if (m_mainBuffer.zoomFactor == m_viewZoomFactor)
        return;

    // All secondary textures serve as the "background overlay".
    m_secondaryBuffer.clear();
    m_secondaryBuffer = m_mainBuffer;

    // Replace the primary textures with an invalid, dirty texture.
    qreal width = m_svg.defaultSize().width() * m_viewZoomFactor;
    qreal height = m_svg.defaultSize().height() * m_viewZoomFactor;
    int horizontal = (width + TileDim - 1) / TileDim;
    int vertical = (height + TileDim - 1) / TileDim;
    m_mainBuffer.resize(horizontal, vertical);

    m_mainBuffer.zoomFactor = m_viewZoomFactor;
    scheduleUpdate();

    killTimer(m_refreshTimer);
    m_refreshTimer = 0;
}

void GLTiger::updateBackingStore()
{
    // During zooming in and out, do not bother.
    if (m_mainBuffer.zoomFactor != m_viewZoomFactor)
        return;

    // Extend the update range with extra tiles in every direction, this is
    // to anticipate panning and scrolling.
    QRect updateRange = m_mainBuffer.visibleRange(m_viewOffset, m_viewZoomFactor, size());
    updateRange.adjust(-ExtraTiles, -ExtraTiles, ExtraTiles, ExtraTiles);

    // Collect all visible tiles which need update.
    QList<QPoint> dirtyTiles;
    for (int x = 0; x < m_mainBuffer.width(); ++x) {
        for (int y = 0; y < m_mainBuffer.height(); ++y) {
            if (m_mainBuffer.at(x, y) == 0 && updateRange.contains(x, y))
                dirtyTiles += QPoint(x, y);
        }
    }

    if (!dirtyTiles.isEmpty()) {

        // Find the closest tile to the center (using Manhattan distance)
        int updateX = dirtyTiles.at(0).x();
        int updateY = dirtyTiles.at(0).x();
        qreal closestDistance = 1e6;
        for (int i = 0; i < dirtyTiles.count(); ++i) {
            int tx = dirtyTiles.at(i).x();
            int ty = dirtyTiles.at(i).y();
            qreal dim = TileDim * m_viewZoomFactor / m_mainBuffer.zoomFactor;
            qreal cx = m_viewOffset.x() + dim * (0.5 + tx);
            qreal cy = m_viewOffset.y() + dim * (0.5 + ty);
            qreal dist = qAbs(cx - width() / 2) + qAbs(cy - height() / 2);
            if (dist < closestDistance) {
                updateX = tx;
                updateY = ty;
                closestDistance = dist;
            }
        }

        // Update the closest tile and bind as texture.
        QImage content(TileDim, TileDim, QImage::Format_ARGB32);
        content.fill(qRgb(255, 255, 255));
        QPainter p(&content);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.translate(-updateX * TileDim, -updateY * TileDim);
        p.scale(m_mainBuffer.zoomFactor, m_mainBuffer.zoomFactor);
        m_svg.render(&p, QRect(QPoint(0, 0), m_svg.defaultSize()));
        p.end();
#ifdef TILE_DEBUG
        p.begin(&content);
        p.drawRect(3, 3, TileDim - 6, TileDim - 6);
        p.drawText(4, 16, QString("%1 %2").arg(updateX).arg(updateY));
        p.end();
#endif
        m_mainBuffer.replace(updateX, updateY, bindTexture(content));
        update();
    }

    killTimer(m_updateTimer);
    m_updateTimer = 0;
}

void GLTiger::initializeGL()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    m_defaultTexture = bindTexture(createCheckerboardPattern(TileDim));
    m_svg.load(QString(":/tiger.svg"));
    refreshBackingStore();
}

void GLTiger::paintGL()
{
    setWindowTitle(QString("GLTiger - Zoom %1%").arg(qRound(m_viewZoomFactor * 100)));

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // For very fast zooming in and out, do not apply any expensive filter.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Ensure we would have seamless transition between adjecent tiles.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // This is background, i.e. all the outdated tiles while
    // new primary ones are being prepared (e.g. after zooming).
    if (!m_secondaryBuffer.isEmpty()) {
        QRect backgroundRange = m_secondaryBuffer.visibleRange(m_viewOffset, m_viewZoomFactor, size());
        m_secondaryBuffer.setViewModelMatrix(m_viewOffset, m_viewZoomFactor);

        for (int x = 0; x < m_secondaryBuffer.width(); ++x) {
            for (int y = 0; y < m_secondaryBuffer.height(); ++y) {
                if (backgroundRange.contains(x, y))
                    m_secondaryBuffer.draw(x, y, m_defaultTexture);
                else
                    m_secondaryBuffer.remove(x, y);
            }
        }
    }

    // Extend the update range with extra tiles in every direction, this is
    // to anticipate panning and scrolling.
    QRect updateRange = m_mainBuffer.visibleRange(m_viewOffset, m_viewZoomFactor, size());
    updateRange.adjust(-ExtraTiles, -ExtraTiles, ExtraTiles, ExtraTiles);

    // When zooming in/out, we have secondary textures as
    // the background. Thus, do not overdraw the background
    // with the checkerboard pattern (default texture).
    GLuint substitute = m_secondaryBuffer.isEmpty() ? m_defaultTexture : 0;

    m_mainBuffer.setViewModelMatrix(m_viewOffset, m_viewZoomFactor);
    bool needsUpdate = false;
    for (int x = 0; x < m_mainBuffer.width(); ++x) {
        for (int y = 0; y < m_mainBuffer.height(); ++y) {
            GLuint texture = m_mainBuffer.at(x, y);
            if (updateRange.contains(x, y)) {
                m_mainBuffer.draw(x, y, substitute);
                if (texture == 0)
                    needsUpdate = true;
            }

            // Save GPU memory and throw out unneeded texture
            if (texture != 0 && !updateRange.contains(x, y))
                m_mainBuffer.remove(x, y);
        }
    }

    if (needsUpdate) {
        scheduleUpdate();
    } else {
        // Every tile is up-to-date, thus discard the background.
        if (!m_secondaryBuffer.isEmpty()) {
            m_secondaryBuffer.clear();
            update();
        }
    }

    // Zooming means we need a fresh set of resolution-correct tiles.
    if (m_viewZoomFactor != m_mainBuffer.zoomFactor)
        scheduleRefresh();
}

void GLTiger::resizeGL(int width, int height)
{
    // Ensure that (0,0) is top left and (width - 1, height -1) is bottom right.
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLTiger::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimer)
        updateBackingStore();
    if (event->timerId() == m_refreshTimer)
        refreshBackingStore();
}

void GLTiger::mouseDoubleClickEvent(QMouseEvent *event)
{
    qreal scale = (event->modifiers() & Qt::ControlModifier) ? 0.5 : 2.0;
    mouseZoom(m_viewZoomFactor * scale, event->pos());
}

void GLTiger::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        m_mousePressPosition = event->pos();
}

void GLTiger::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_viewOffset += (event->pos() - m_mousePressPosition);
        m_mousePressPosition = event->pos();
        update();
    }
}

void GLTiger::mouseZoom(qreal zoomFactor, QPoint pos)
{
    qreal oldZoom = m_viewZoomFactor;
    m_viewZoomFactor = qBound(MinZoom, zoomFactor, MaxZoom);

    // We center the zooming relative to the mouse positions,
    // hence the translation before and after the scaling.
    QPointF center = pos - m_viewOffset;
    center *= (m_viewZoomFactor / oldZoom);
    m_viewOffset = pos - center;

    update();
}

void GLTiger::wheelEvent(QWheelEvent *event)
{
    qreal dz = event->delta() / 100.0f;
    mouseZoom(m_viewZoomFactor + dz, event->pos());
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GLTiger window;
    window.resize(640, 480);
    window.show();
    return app.exec();
}
