/*
    This file is part of the OfiLabs X2 project.

    Copyright (C) 2010 Helder Correia <helder.pereira.correia@gmail.com>

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
#include <cmath>

class PlasmaEffect : public QWidget
{
    Q_OBJECT

public:
    typedef double (*transformation)(double);
    enum BaseColor { Red, Green, Blue };
    PlasmaEffect(int width = 640, int height = 360, QWidget *parent = 0);
    int interval() const;

public slots:
    void setBaseColor(BaseColor color);
    void setParameters(qreal alpha, qreal alphaAdjust, qreal beta, qreal betaAdjust);
    void setBaseFunction(transformation function);
    void start();
    void stop();
    void setInterval(int msec);
    void toggleAnimation();
    void toggleFullScreen();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void timerEvent(QTimerEvent *event);

private:
    int m_plasmaWidth;
    int m_plasmaHeight;
    bool m_fullScreen;

    QImage m_image;
    int *m_pattern;
    QVector<QRgb> m_palette;
    int m_timerInterval;
    QBasicTimer m_animationTimer;
    transformation m_baseFunction;

    qreal m_alpha;
    qreal m_alphaAdjust;
    qreal m_beta;
    qreal m_betaAdjust;

    int m_redComponent;
    int m_greenComponent;
    int m_blueComponent;

    int m_redComponentChangeFactor;
    int m_greenComponentChangeFactor;
    int m_blueComponentChangeFactor;

    void paintNextFrame();
    void setUp();
};

PlasmaEffect::PlasmaEffect(int width, int height, QWidget *parent) : QWidget(parent),
    m_plasmaWidth(width), m_plasmaHeight(height), m_fullScreen(false),
    m_pattern(0), m_palette(256),
    m_timerInterval(40), m_baseFunction(sin),
    m_alpha(20), m_alphaAdjust(0.15), m_beta(100), m_betaAdjust(0.015),
    m_redComponent(0), m_greenComponent(255), m_blueComponent(0),
    m_redComponentChangeFactor(2), m_greenComponentChangeFactor(-2),
    m_blueComponentChangeFactor(2)
{
    setAttribute(Qt::WA_StaticContents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    resize(width, height);
}

int PlasmaEffect::interval() const
{
    return m_timerInterval;
}

void PlasmaEffect::setBaseColor(BaseColor color)
{
    if (color == Red) {
        m_redComponent   = 255; m_redComponentChangeFactor   = -2;
        m_greenComponent = 0;   m_greenComponentChangeFactor = 2;
        m_blueComponent  = 0;   m_blueComponentChangeFactor  = 2;
    } else if (color == Green) {
        m_redComponent   = 0;   m_redComponentChangeFactor   = 2;
        m_greenComponent = 255; m_greenComponentChangeFactor = -2;
        m_blueComponent  = 0;   m_blueComponentChangeFactor  = 2;
    } else {
        m_redComponent   = 0;   m_redComponentChangeFactor   = 2;
        m_greenComponent = 0;   m_greenComponentChangeFactor = 2;
        m_blueComponent  = 255; m_blueComponentChangeFactor  = -2;
    }
    setUp();
}

void PlasmaEffect::setParameters(qreal alpha, qreal alphaAdjust, qreal beta, qreal betaAdjust)
{
    m_alpha = alpha; m_alphaAdjust = alphaAdjust;
    m_beta = beta; m_betaAdjust = betaAdjust;
    setUp();
}

void PlasmaEffect::setBaseFunction(transformation function)
{
    m_baseFunction = function;
    setUp();
}

void PlasmaEffect::start()
{
    if (!m_animationTimer.isActive())
        m_animationTimer.start(interval(), this);
}

void PlasmaEffect::stop()
{
    if (m_animationTimer.isActive())
        m_animationTimer.stop();
}

void PlasmaEffect::setInterval(int msec)
{
    if (msec > 250 || msec < 10)
        return;
    if (m_animationTimer.isActive())
        m_animationTimer.stop();
    m_timerInterval = msec;
    m_animationTimer.start(msec, this);
}

void PlasmaEffect::toggleAnimation()
{
    if (m_animationTimer.isActive())
        m_animationTimer.stop();
    else
        m_animationTimer.start(interval(), this);
}

void PlasmaEffect::toggleFullScreen()
{
    if (m_fullScreen) {
        showNormal();
        m_fullScreen = false;
    } else {
        showFullScreen();
        m_fullScreen = true;
    }
}

void PlasmaEffect::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {

    case Qt::Key_Left:
        if (event->modifiers() & Qt::ShiftModifier)
            m_betaAdjust -= 0.0008;
        else
            m_alphaAdjust -= 0.002;
        setUp();
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ShiftModifier)
            m_betaAdjust += 0.0008;
        else
            m_alphaAdjust += 0.002;
        setUp();
        break;

    case Qt::Key_Up:
        if (event->modifiers() & Qt::ShiftModifier)
            m_beta += 8;
        else
            m_alpha += 2;
        setUp();
        break;
    case Qt::Key_Down:
        if (event->modifiers() & Qt::ShiftModifier)
            m_beta -= 8;
        else
            m_alpha -= 2;
        setUp();
        break;

    case Qt::Key_R: setBaseColor(Red); break;
    case Qt::Key_G: setBaseColor(Green); break;
    case Qt::Key_B: setBaseColor(Blue); break;

    case Qt::Key_S: setBaseFunction(sin); break;
    case Qt::Key_T: setBaseFunction(tan); break;

    case Qt::Key_F: toggleFullScreen(); break;
    case Qt::Key_Space: toggleAnimation(); break;

    case Qt::Key_O: setInterval(interval() + 5); break;
    case Qt::Key_P: setInterval(interval() - 5); break;

    case Qt::Key_1: setParameters(0, -0.046, -4, 0.0086); break;
    case Qt::Key_2: setParameters(-16, -0.012, 4, -0.0266); break;
    case Qt::Key_3: setParameters(-58, -0.002, 4, -0.1146); break;
    case Qt::Key_4: setParameters(-58, -0.002, -4, -0.0138); break;
    case Qt::Key_5: setParameters(4, 0.102, 12, -0.0686); break;
    case Qt::Key_6: setParameters(-2, 0.244, 1780, -0.0007); break;
    case Qt::Key_7: setParameters(-34, 0.092, 1780, -0.0007); break;
    case Qt::Key_8: setParameters(-130, 0.008, -12, -0.043); break;
    case Qt::Key_9: setParameters(-38, 0.002, 12, -0.0662); break;
    case Qt::Key_0: setParameters(20, 0.15, 100, 0.015); break;

    case Qt::Key_Escape: QApplication::quit(); break;
    default: QWidget::keyPressEvent(event);
    }
}

void PlasmaEffect::mousePressEvent(QMouseEvent *)
{
    toggleFullScreen();
}

void PlasmaEffect::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QPointF(), m_image);
}

void PlasmaEffect::resizeEvent(QResizeEvent *event)
{
    m_plasmaWidth = event->size().width();
    m_plasmaHeight = event->size().height();
    m_image = QImage(m_plasmaWidth, m_plasmaHeight, QImage::Format_RGB32);

    if (m_pattern)
        delete m_pattern;
    m_pattern = new int[m_plasmaWidth*m_plasmaHeight];

    setUp();

    if (!m_animationTimer.isActive())
        paintNextFrame();
}

void PlasmaEffect::timerEvent(QTimerEvent *)
{
    paintNextFrame();
}

void PlasmaEffect::paintNextFrame()
{
    for (int i = 0; i < 255; ++i)
        m_palette[i] = m_palette[i + 1];
    m_palette[255] = m_palette[0];

    QRgb *bits = reinterpret_cast<QRgb*>(m_image.bits());
    int *p = m_pattern;
    for (int y = 0; y < m_plasmaHeight; ++y)
        for (int x = 0; x < m_plasmaWidth; ++x)
            *bits++ = m_palette[*p++];

    update();
}

void PlasmaEffect::setUp()
{
    int maxDimension = qMax(m_plasmaWidth, m_plasmaHeight);
    int *f = new int[maxDimension];

    for (int x = 0; x < maxDimension; ++x)
        f[x] = qRound(m_alpha * m_baseFunction(x * m_alphaAdjust) + m_beta * cos(x * m_betaAdjust));

    int r = m_redComponent, g = m_greenComponent, b = m_blueComponent;
    for (int i = 0; i < 128; ++i) {
        m_palette[i] = m_palette[255-i] = qRgb(r, g, b);
        r += m_redComponentChangeFactor;
        g += m_greenComponentChangeFactor;
        b += m_blueComponentChangeFactor;
    }

    for (int y = 0; y < m_plasmaHeight; ++y)
        for (int x = 0; x < m_plasmaWidth; ++x)
            m_pattern[y * m_plasmaWidth + x] = abs(f[x] + f[y]) % (255);

    delete[] f;
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    PlasmaEffect plasma;
#ifdef Q_OS_SYMBIAN
    plasma.showMaximized(); // http://bugreports.qt.nokia.com/browse/QTBUG-8190
#else
    plasma.show();
#endif
    plasma.start();

    return application.exec();
}

#include "plasmaeffect.moc"

