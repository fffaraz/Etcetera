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

#ifndef CODEMIRROR
#define CODEMIRROR

#include <QWebView>

class External: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString data READ data)

public:
    External(QObject *parent = 0)
        : QObject(parent)
    {
    }

    QString text;
    QString data() const {
        return text;
    }
};

class CodeMirror: public QWebView
{
    Q_OBJECT

public:
    CodeMirror(QWidget *parent = 0);

    QString text() const;
    void setText(const QString &text);

    void changeTheme(const QString &theme);

public slots:
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

private:
    External *m_external;
};

#endif
