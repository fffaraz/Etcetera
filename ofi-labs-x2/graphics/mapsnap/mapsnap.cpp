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

// If defined, use the tile images hosted by MapQuest instead of OpenStreetMap
#define USE_MAPQUEST

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_CONNECTION 6

#ifdef USE_MAPQUEST
// http://wiki.openstreetmap.org/wiki/Mapquest
const char *tileURL = "http://otile1.mqcdn.com/tiles/1.0.0/osm/%1/%2/%3.png";
const char *attribution = "Data, imagery and map information provided by MapQuest, OpenStreetMap and contributors, CC-BY-SA";
#else
// http://wiki.openstreetmap.org/wiki/Slippy_Map#Mapnik_tile_rendering
const char *tileURL = "http://tile.openstreetmap.org/%1/%2/%3.png";
const char *attribution = "(c) OpenStreetMap (and) contributors, CC-BY-SA";
#endif

#if QT_VERSION < 0x040700
uint qHash(const QUrl &url)
{
    return qHash(url.toEncoded());
}
#endif

class MapModel: public QObject
{
    Q_OBJECT

public:
    MapModel(QObject *parent = 0);

    bool draw(QPainter *painter, qreal latitude, qreal longitude,
              int zoomLevel, int viewportWidth, int viewportHeight);

signals:
    void updated();

protected slots:
    void download();

private slots:
    void updateTile();

private:
    int m_tileSize;
    QNetworkAccessManager m_manager;
    QTimer m_updateTimer;
    QHash<QUrl, QNetworkReply*> m_tileReplies;
    QStringList m_pendingList;
    QHash<QString, QImage> m_tileImages;
};

MapModel::MapModel(QObject *parent)
    : QObject(parent)
    , m_tileSize(256)
{
    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(100);
    connect(&m_updateTimer, SIGNAL(timeout()), SLOT(download()));
}

bool MapModel::draw(QPainter *painter, qreal latitude, qreal longitude,
                    int zoomLevel, int viewportWidth, int viewportHeight)
{
    // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Zoom_levels
    if (zoomLevel < 1 || zoomLevel > 17)
        return true;

    // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#C.2FC.2B.2B
    int zt = 1 << zoomLevel;
    qreal tx = zt * (longitude + 180.0) / 360.0;
    qreal ty = zt * (1.0 - log(tan(latitude * M_PI / 180.0) +
                               1.0 / cos(latitude * M_PI / 180.0)) / M_PI) / 2.0;

    int ofsx = viewportWidth / 2 - (tx - floor(tx)) * m_tileSize;
    int ofsy = viewportHeight / 2 - (ty - floor(ty)) * m_tileSize;
    int xa = (ofsx + m_tileSize - 1) / m_tileSize;
    int ya = (ofsy + m_tileSize - 1) / m_tileSize;
    ofsx -= xa * m_tileSize;
    ofsy -= ya * m_tileSize;

    int x1 = static_cast<int>(tx) - xa;
    int y1 = static_cast<int>(ty) - ya;
    int x2 = static_cast<int>(tx) + (viewportWidth - ofsx - 1) / m_tileSize;
    int y2 = static_cast<int>(ty) + (viewportHeight - ofsy - 1) / m_tileSize;

    bool complete = true;
    for (int tpx = x1; tpx <= x2; ++tpx)
        for (int tpy = y1; tpy <= y2; ++tpy) {
            int ax = ((tpx % zt) + zt) % zt;
            int ay = ((tpy % zt) + zt) % zt;
            QString key = QString("%1/%2/%3").arg(zoomLevel).arg(ax).arg(ay);
            if (m_tileImages.contains(key)) {
                int posx = (tpx - x1) * m_tileSize + ofsx;
                int posy = (tpy - y1) * m_tileSize + ofsy;
                painter->drawImage(posx, posy, m_tileImages.value(key));
            } else {
                complete = false;
                m_pendingList += key;
                m_updateTimer.stop();
                m_updateTimer.start();
            }
        }

    return complete;
}

void MapModel::download()
{
    while (!m_pendingList.isEmpty() && m_tileReplies.count() < MAX_CONNECTION) {
        QString key = m_pendingList.first();
        m_pendingList.removeFirst();
        QStringList values = key.split('/');
        if (values.count() == 3) {
            int zoomLevel = values.at(0).toInt();
            int tileX = values.at(1).toInt();
            int tileY = values.at(2).toInt();
            QString path = QString::fromLatin1(tileURL);
            QUrl url = QUrl(path.arg(zoomLevel).arg(tileX).arg(tileY));
            if (!url.isEmpty() && !m_tileReplies.contains(url)) {
                QNetworkRequest request;
                request.setUrl(url);
                request.setRawHeader("User-Agent", "X2 from Ofi Labs");
                request.setAttribute(QNetworkRequest::User, key);
                QNetworkReply *reply = m_manager.get(request);
                connect(reply, SIGNAL(finished()), this, SLOT(updateTile()));
                m_tileReplies[url] = reply;
            }
        }
    }
}

void MapModel::updateTile()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    m_tileReplies.remove(reply->url());
    if (reply->error()) {
        qWarning() << "Error for" << reply->url() << reply->errorString();
        m_tileReplies[reply->url()] = 0;
    } else {
        QImage image;
        if (image.load(reply, 0)) {
            QString key = reply->request().attribute(QNetworkRequest::User).toString();
            m_tileImages.insert(key, image);
        } else {
            qWarning() << "Can't decode tile image of" << reply->url();
        }
    }
    reply->deleteLater();

    emit updated();
}

class MapSnap: public QObject
{
    Q_OBJECT

public:
    MapSnap(QObject *parent = 0);

    qreal latitude;
    qreal longitude;
    int zoomLevel;
    QImage buffer;

public slots:
    void update();

signals:
    void completed();

private:
    MapModel model;
};

MapSnap::MapSnap(QObject *parent)
    : QObject(parent)
{
    connect(&model, SIGNAL(updated()), SLOT(update()));
}

void MapSnap::update()
{
    QPainter painter;
    painter.begin(&buffer);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    bool complete = model.draw(&painter, latitude, longitude, zoomLevel,
                               buffer.width(), buffer.height());
    painter.end();

    painter.begin(&buffer);
    int flags =  Qt::AlignBottom | Qt::AlignHCenter | Qt::TextWordWrap;
    int bw = buffer.width();
    QRect textArea = buffer.rect();
    textArea.setWidth(bw * 0.7);
    textArea = painter.boundingRect(textArea, flags, attribution);
    textArea.translate((bw - textArea.width()) / 2, 0);
    painter.setBrush(QColor(255, 255, 255, 128));
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, textArea.y(), bw, textArea.height());
    painter.setPen(QColor(32, 32, 32));
    for (int x = -2; x <= 2; ++x)
        for (int y = -2; y <= 2; ++y)
            painter.drawText(textArea.translated(x, y), flags, attribution);
    painter.setPen(Qt::white);
    painter.drawText(textArea, flags, attribution);
    painter.end();

    if (complete)
        emit completed();
}

#include "mapsnap.moc"

#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (argc < 5) {
        std::cout << "mapsnap latitude longitude zoom filename [width [height]]";
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Example:";
        std::cout << std::endl;
        std::cout << "mapsnap 37.45108 -122.15917 12 output.png 600 450";
        std::cout << std::endl;
        return -1;
    }

    int w = (argc > 5) ? QString(argv[5]).toInt() : 600;
    int h = (argc > 6) ? QString(argv[6]).toInt() : 450;

    MapSnap snap;
    QObject::connect(&snap, SIGNAL(completed()), qApp, SLOT(quit()));
    snap.latitude = QString(argv[1]).toDouble();
    snap.longitude = QString(argv[2]).toDouble();
    snap.zoomLevel = QString(argv[3]).toInt();
    snap.buffer = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
    snap.update();

    app.exec();
    snap.buffer.save(argv[4]);

    return 0;
}
