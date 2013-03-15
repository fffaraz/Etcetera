/*
    This file is part of the OfiLabs X2 project.

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

struct role_t {
    const char *name;
    QPalette::ColorRole role;
};

role_t paletteRoles[] =  {
    { "Window",          QPalette::Window },
    { "WindowText",      QPalette::WindowText },
    { "Base",            QPalette::Base },
    { "AlternateBase",   QPalette::AlternateBase },
    { "Text",            QPalette::Text },
    { "Button",          QPalette::Button },
    { "ButtonText",      QPalette::ButtonText },
    { "BrightText",      QPalette::BrightText },
    { "Light",           QPalette::Light },
    { "Midlight",        QPalette::Midlight },
    { "Dark",            QPalette::Dark },
    { "Mid",             QPalette::Mid },
    { "Shadow",          QPalette::Shadow },
    { "Highlight",       QPalette::Highlight },
    { "HighlightedText", QPalette::HighlightedText },
    { "Link",            QPalette::Link },
    { "LinkVisited",     QPalette::LinkVisited }
};

struct group_t {
    const char *name;
    QPalette::ColorGroup group;
};

group_t paletteGroups[] = {
    { "Active",    QPalette::Active },
    { "Inactive",  QPalette::Inactive },
    { "Disabled",  QPalette::Disabled },
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    int roleCount = sizeof(paletteRoles) / sizeof(paletteRoles[0]);
    int groupCount = sizeof(paletteGroups) / sizeof(paletteGroups[0]);

    QTableWidget widget;
    widget.setSelectionMode(QTableWidget::NoSelection);
    widget.setSortingEnabled(false);
    widget.verticalHeader()->hide();
    widget.horizontalHeader()->hide();
    widget.setRowCount(roleCount);
    widget.setColumnCount(groupCount + 1);
    widget.setFrameStyle(QFrame::NoFrame);

    QPalette palette = widget.palette();

    for (int c = 0; c < roleCount; ++c) {
        QString name = QString::fromLatin1(paletteRoles[c].name);
        widget.setItem(c, 0, new QTableWidgetItem(name));
        for (int i = 0; i < groupCount; ++i) {
            QColor color = palette.color(paletteGroups[i].group, paletteRoles[c].role);
            QPixmap pixmap(32, 32);
            QPainter painter(&pixmap);
            painter.fillRect(pixmap.rect(), Qt::black);
            painter.fillRect(pixmap.rect().adjusted(1, 1, -1, -1), color);
            QString rgb;
            rgb.sprintf("%03d %03d %03d", color.red(), color.green(), color.blue());
            widget.setItem(c, i + 1, new QTableWidgetItem(QIcon(pixmap), rgb));
        }
    }

    widget.setWindowTitle("Palette Viewer");
    widget.resizeColumnsToContents();
    widget.show();

    return app.exec();
}

