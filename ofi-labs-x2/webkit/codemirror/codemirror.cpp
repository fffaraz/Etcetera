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

#include "codemirror.h"

#include <QtWebKit>

CodeMirror::CodeMirror(QWidget *parent)
    : QWebView(parent)
{
    settings()->setFontFamily(QWebSettings::FixedFont, "Monaco, Monospace");

    load(QUrl("qrc:/index.html"));
    changeTheme("default");

    m_external = new External(this);
    page()->mainFrame()->addToJavaScriptWindowObject("External", m_external);
}

QString CodeMirror::text() const
{
    return page()->mainFrame()->evaluateJavaScript("editor.getValue()").toString();
}

void CodeMirror::setText(const QString &text)
{
    m_external->text = text;
    page()->mainFrame()->evaluateJavaScript("editor.setValue(External.data)");
}

void CodeMirror::changeTheme(const QString &theme)
{
    QWebFrame *frame = page()->mainFrame();
    frame->evaluateJavaScript(QString("editor.setOption('theme', '%1')").arg(theme));
    frame->evaluateJavaScript(QString("document.body.className = 'cm-s-%1'").arg(theme));
}

void CodeMirror::undo()
{
    page()->mainFrame()->evaluateJavaScript("editor.undo()");
}

void CodeMirror::redo()
{
    page()->mainFrame()->evaluateJavaScript("editor.redo()");
}

void CodeMirror::cut()
{
    page()->triggerAction(QWebPage::Cut);
}

void CodeMirror::copy()
{
    page()->triggerAction(QWebPage::Copy);
}

void CodeMirror::paste()
{
    page()->triggerAction(QWebPage::Paste);
}

