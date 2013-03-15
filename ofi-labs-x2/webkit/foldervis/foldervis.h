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

#ifndef FOLDERVIS
#define FOLDERVIS

#include <QObject>
#include <QWebView>

struct Entry {
    QString name;
    int size;
    QList<Entry> children;
};

class Crawler: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString tree READ tree)

public:
    Crawler(QObject *parent);
    QString tree() const;

public slots:
    void start(const QString &dir);

signals:
    void progress(int count);
    void finished();

private:
    QString m_dir;
    int m_count;
    Entry m_rootEntry;

protected:
    Entry search(const QString &dir);
};

class Visualizer: public QWebView
{
    Q_OBJECT

public:
    Visualizer(const QString &dir);

private slots:
    void setup();

private:
    QString m_dir;
    Crawler *m_crawler;
};

#endif
