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

#include "foldervis.h"

#include <QtGui>
#include <QtWebKit>


Crawler::Crawler(QObject *parent)
    : QObject(parent)
    , m_count(0)
{
}

void Crawler::start(const QString &dir)
{
    m_dir = dir;
    m_count = 0;
    m_rootEntry = search(m_dir);
    emit finished();
}

Entry Crawler::search(const QString &path)
{
    QList<Entry> children;

    int total = 0;

    m_count++;
    emit progress(m_count);
    QApplication::processEvents();

    QFileInfoList list = QDir(path).entryInfoList();
    for (int i = 0; i < list.count(); ++i) {
        Entry entry;
        QFileInfo fi = list.at(i);
        if (fi.fileName() == "." || fi.fileName() == "..")
            continue;
        if (fi.isDir() && fi.baseName() != ".") {
            entry = search(fi.absoluteFilePath());
        } else {
            entry.name = fi.fileName();
            entry.size = fi.size();
        }
        total += entry.size;
        children.append(entry);
    }

    Entry entry;
    entry.name = QFileInfo(path).fileName();
    entry.children = children;
    entry.size = total;

    return entry;
}


QString displayedSize(int size)
{
    const qreal K = 1024;
    const qreal M = K * K;
    const qreal G = K * M;

    if (size > 1024 * 1024 * 1024) {
        return QString::number(size / G, 'f', 1) + "G";
    }
    if (size > 1024 * 1024) {
        return QString::number(size / M, 'f', 1) + "M";
    }
    if (size > 1024) {
        return QString::number(size / K, 'f', 1) + "K";
    }
    return QString::number(size);
}

QString stringified(const Entry &entry)
{
    QString result = "{";
    result += QString("\"name\": \"%1 %2\", ").arg(entry.name).arg(displayedSize(entry.size));
    result += QString("\"id\": \"%1\", ").arg(entry.name);
    result += QString("\"data\": { \"$area\": %1 }, ").arg(entry.size);
    result += "\"children\": [";
    for (int i = 0; i < entry.children.count(); ++i) {
        result += stringified(entry.children.at(i));
        if (i < entry.children.count() - 1)
            result += ',';
    }
    result += "]";
    result += "}";
    return result;
}

QString Crawler::tree() const
{
    return stringified(m_rootEntry);
}

Visualizer::Visualizer(const QString &dir)
    : QWebView(0)
    , m_dir(dir)
    , m_crawler(new Crawler(this))
{
    setFixedSize(600, 600);
    load(QUrl("qrc:/index.html"));

    QWebFrame *frame = page()->mainFrame();
    frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    frame->addToJavaScriptWindowObject("crawler", m_crawler);

    QFile file(":/bootstrap.js");
    file.open(QFile::ReadOnly);
    QString bootstrap = file.readAll();
    file.close();
    page()->mainFrame()->evaluateJavaScript(bootstrap);

    QTimer::singleShot(250, this, SLOT(setup()));
}

void Visualizer::setup()
{
    m_crawler->start(m_dir);
}

int main(int argc, char **argv)
{
    QApplication::setGraphicsSystem("raster");
    QApplication app(argc, argv);

    QString dir = QDir::homePath();
    if (argc > 1)
      dir = QString::fromLocal8Bit(argv[1]);

    Visualizer visualizer(dir);
    visualizer.setWindowTitle("Folder Visualizer");
    visualizer.show();

    return app.exec();
}
