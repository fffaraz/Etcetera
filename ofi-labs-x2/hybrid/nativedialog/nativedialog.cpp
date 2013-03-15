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

#include "nativedialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QWebFrame>
#include <QWebView>

Dialog::Dialog(QObject *parent)
    : QObject(parent)
{
}

void Dialog::showMessage(const QString &msg)
{
    QMessageBox::information(0, "Information", msg);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWebView webView;
    webView.resize(400, 300);
    webView.setWindowTitle("Dialog Example");
    webView.show();

    webView.load(QUrl("qrc:/index.html"));
    QWebFrame *mainFrame = webView.page()->mainFrame();
    mainFrame->addToJavaScriptWindowObject("Dialog", new Dialog);

    return app.exec();
}
