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

#include "editor.h"
#include "codemirror.h"

#include <QtGui>

Editor::Editor(QWidget *parent)
    : QMainWindow(parent)
    , m_edit(0)
{
    m_edit = new CodeMirror(this);
    setCentralWidget(m_edit);

    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New Window", this, SLOT(fileNew()), QKeySequence::New);
    fileMenu->addAction("&Open...", this, SLOT(fileOpen()), QKeySequence::Open);
    fileMenu->addAction("&Save", this, SLOT(fileSave()), QKeySequence::Save);
    fileMenu->addAction("Save &As...", this, SLOT(fileSaveAs()));

    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Undo", m_edit, SLOT(undo()), QKeySequence::Undo);
    editMenu->addAction("&Redo", m_edit, SLOT(redo()), QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction("Cu&t", m_edit, SLOT(cut()), QKeySequence::Cut);
    editMenu->addAction("&Copy", m_edit, SLOT(copy()), QKeySequence::Copy);
    editMenu->addAction("&Paste", m_edit, SLOT(paste()), QKeySequence::Paste);

    QMenu *themeMenu = menuBar()->addMenu("&Theme");
    themeMenu->addAction("Default", this, SLOT(themeDefault()));
    themeMenu->addSeparator();
    themeMenu->addAction("Cobalt", this, SLOT(themeCobalt()));
    themeMenu->addAction("Elegant", this, SLOT(themeElegant()));
    themeMenu->addAction("Neat", this, SLOT(themeNeat()));
    themeMenu->addAction("Night", this, SLOT(themeNight()));

    m_fileName = QString();
    updateTitle();
}

void Editor::fileNew()
{
    Editor *editor = new Editor(parentWidget());
    editor->show();
}

void Editor::fileOpen()
{
    if (isModified()) {
        if (QMessageBox::question(this, "Modified", "Do you want to save the file first?",
            QMessageBox::Save, QMessageBox::No) == QMessageBox::Save) {
            if (!fileSaveAs())
                return;
        }
    }
    QString inputName = QFileDialog::getOpenFileName(this, "Open File", QString(), "JavaScript (*.js)");
    if (inputName.isEmpty())
        return;

    QFile file(inputName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, "Open Error", QString("Can not open %1 for reading!").arg(inputName));
        return;
    }
    QString content = file.readAll();
    file.close();
    m_fileName = inputName;
    m_content = content;
    m_edit->setText(content);
    updateTitle();
}

void Editor::fileSave()
{
    if (m_fileName.isEmpty()) {
        fileSaveAs();
        return;
    }
    save(m_fileName);
}

bool Editor::fileSaveAs()
{
    QString outputName = QFileDialog::getSaveFileName(this, "Save File", m_fileName, "JavaScript (*.js)");
    if (outputName.isEmpty())
        return false;
    save(outputName);
    return true;
}

void Editor::themeDefault()
{
    m_edit->changeTheme("default");
}

void Editor::themeCobalt()
{
    m_edit->changeTheme("cobalt");
}

void Editor::themeElegant()
{
    m_edit->changeTheme("elegant");
}

void Editor::themeNeat()
{
    m_edit->changeTheme("neat");
}

void Editor::themeNight()
{
    m_edit->changeTheme("night");
}

bool Editor::isModified()
{
    return m_edit->text() != m_content;
}

void Editor::save(const QString &outputName)
{
    QFile file(outputName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, "Save Error", QString("Can not open %1 for writing!").arg(outputName));
        return;
    }
    m_content = m_edit->text();
    file.write(m_content.toUtf8());
    file.close();

    m_fileName = outputName;
    updateTitle();
}

void Editor::updateTitle()
{
    QString name = m_fileName;
    if (name.isEmpty())
        name = "Untitled";
    else
        name = QFileInfo(m_fileName).fileName();
    setWindowTitle(QString("%1 - CodeMirror Desktop").arg(name));
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("CodeMirror Desktop");
    Editor editor;
    editor.show();
    return app.exec();
}
