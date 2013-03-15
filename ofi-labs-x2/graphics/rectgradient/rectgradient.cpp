/*
  This file is part of the Ofi Labs X2 project.

  Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QtGui>

QImage createRectGradient(qreal width, qreal height, const QGradient &gradient)
{
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
    QImage buffer(qCeil(width), qCeil(height), format);
    buffer.fill(qRgba(255, 255, 255, 0));

    QPainter painter(&buffer);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Divide the rectangle into four triangles, each facing North, South,
    // East, and West. Every triangle will have its bounding box and linear
    // gradient.
    QLinearGradient linearGradient;

    // from center going North
    linearGradient = QLinearGradient(0, 0, width, height / 2);
    linearGradient.setStops(gradient.stops());
    linearGradient.setStart(width / 2, height / 2);
    linearGradient.setFinalStop(width / 2, 0);
    painter.setBrush(linearGradient);
    painter.drawRect(0, 0, width, height / 2);

    // from center going South
    linearGradient = QLinearGradient(0, 0, width, height / 2);
    linearGradient.setStops(gradient.stops());
    linearGradient.setStart(width / 2, height / 2);
    linearGradient.setFinalStop(width / 2, height);
    painter.setBrush(linearGradient);
    painter.drawRect(0, height / 2, width, height / 2);

    // clip the East and West portion
    QPainterPath clip;
    clip.moveTo(width, 0);
    clip.lineTo(width, height);
    clip.lineTo(0, 0);
    clip.lineTo(0, height);
    clip.closeSubpath();
    painter.setClipPath(clip);

    // from center going East
    linearGradient = QLinearGradient(0, 0, width / 2, height);
    linearGradient.setStops(gradient.stops());
    linearGradient.setStart(width / 2, height / 2);
    linearGradient.setFinalStop(width, height / 2);
    painter.setBrush(linearGradient);
    painter.drawRect(width / 2, 0, width, height);

    // from center going West
    linearGradient = QLinearGradient(0, 0, width / 2, height);
    linearGradient.setStops(gradient.stops());
    linearGradient.setStart(width / 2, height / 2);
    linearGradient.setFinalStop(0, height / 2);
    painter.setBrush(linearGradient);
    painter.drawRect(0, 0, width / 2, height);

    painter.end();
    return buffer;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QGradient gradient;
    gradient.setColorAt(1, Qt::black);
    gradient.setColorAt(0.4, Qt::darkGray);
    gradient.setColorAt(0, Qt::white);
    createRectGradient(400, 150, gradient).save("gradient.png");

    return 0;
}
