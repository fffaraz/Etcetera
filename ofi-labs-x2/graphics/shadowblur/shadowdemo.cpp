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
#include <QtNetwork>

#include "ui_parameters.h"
#include "shadowblur.h"

class ShadowDemo: public QWidget
{
    Q_OBJECT

public:
    ShadowDemo(QWidget *parent = 0);
    void loadImage(const QString &file);
    void loadImage(const QImage &image);

protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent *event);
    void paintEvent(QPaintEvent*);

public slots:
    void handleNetworkData(QNetworkReply*);

private:
    Ui::ParametersForm parameters;
    QNetworkAccessManager m_networkManager;
    QImage m_image;
    QImage m_shadow;
    QString m_fileName;
};

ShadowDemo::ShadowDemo(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);

    setAttribute(Qt::WA_StaticContents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    connect(&m_networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));

    QWidget* toolWidget = new QWidget(this);
    toolWidget->setWindowFlags(Qt::Tool | Qt::WindowTitleHint);
    parameters.setupUi(toolWidget);
    toolWidget->show();
    toolWidget->adjustSize();

    connect(parameters.blurRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.redSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.greenSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.blueSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.xOffsetSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.yOffsetSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.hideImageCheckBox, SIGNAL(toggled(bool)), this, SLOT(update()));

    parameters.blurRadiusSlider->setValue(11);
    parameters.redSlider->setValue(32);
    parameters.greenSlider->setValue(32);
    parameters.blueSlider->setValue(32);
    parameters.xOffsetSlider->setValue(3);
    parameters.yOffsetSlider->setValue(3);
}

void ShadowDemo::handleNetworkData(QNetworkReply *reply)
{
    QImage image;

    QUrl url = reply->url();
    if (reply->error()) {
        m_fileName = QString();
        setWindowTitle(QString("Can not download %1").arg(url.toString()));
    } else {
        m_fileName = url.toString();
        if (url.scheme() == "file")
            m_fileName = url.toLocalFile();
        image.load(reply, 0);
    }

    reply->deleteLater();
    loadImage(image);
}

void ShadowDemo::loadImage(const QImage &image)
{
    m_image = image;
    if (m_image.isNull()) {
        setFixedSize(512, 256);
        setWindowTitle(QString("Can not load %1").arg(m_fileName));
    } else {
        QString title = "Shadow Demo ";
        if ((m_image.width() > 1024) || (m_image.height() > 800)) {
            int w = m_image.width();
            m_image = m_image.scaled(640, 480, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            title += QString("[Zoom %1%] ").arg(m_image.width() * 100 / w);
        }
        m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        setWindowTitle(QString("%1: %2 (%3 x %4)").arg(title).arg(m_fileName).
                       arg(m_image.width()).arg(m_image.height()));
        setFixedSize(m_image.size());
    }
    update();
}

void ShadowDemo::loadImage(const QString &fileName)
{
    m_fileName = QFileInfo(fileName).fileName();
    loadImage(QImage(fileName));
}

void ShadowDemo::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void ShadowDemo::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.count()) {
        QUrl url = urls[0];
        if (event->mimeData()->hasImage()) {
            QImage img = qvariant_cast<QImage>(event->mimeData()->imageData());
            m_fileName = url.toString();
            loadImage(img);
        } else {
            m_networkManager.get(QNetworkRequest(url));
            setWindowTitle(QString("Loading %1...").arg(url.toString()));
        }
        event->acceptProposedAction();
    }
}

void ShadowDemo::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    if (m_image.isNull()) {
        painter.drawText(rect(), Qt::AlignCenter, "Drag and drop an image here.");
    } else {
        int radius = parameters.blurRadiusSlider->value();
        int offsetX = parameters.xOffsetSlider->value();
        int offsetY = parameters.yOffsetSlider->value();

        QColor color;
        color.setRed(parameters.redSlider->value());
        color.setGreen(parameters.greenSlider->value());
        color.setBlue(parameters.blueSlider->value());

        m_shadow = m_image;
        shadowBlur(m_shadow, radius, color);
        painter.drawImage(offsetX, offsetY, m_shadow);

        if (!parameters.hideImageCheckBox->isChecked())
            painter.drawImage(0, 0, m_image);
    }
}

#include "shadowdemo.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ShadowDemo widget;
    widget.resize(400, 300);
    widget.setWindowTitle("Shadow Demo");
    widget.show();
    if (argc > 1)
        widget.loadImage(argv[1]);

    return app.exec();
}
