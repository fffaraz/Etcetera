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

#ifndef EDITOR
#define EDITOR

#include <QMainWindow>

class CodeMirror;

class Editor: public QMainWindow
{
    Q_OBJECT

public:
    Editor(QWidget *parent = 0);

private slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    bool fileSaveAs();
    void themeDefault();
    void themeCobalt();
    void themeElegant();
    void themeNeat();
    void themeNight();

protected:
    bool isModified();
    void save(const QString &outputName);
    void updateTitle();

private:
    QString m_fileName;
    QString m_content;
    CodeMirror *m_edit;
};

#endif
