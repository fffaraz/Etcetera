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

/*global window:true Int16Array:true */

function Crossfader(canvas, image1, image2) {

    var context = canvas.getContext('2d'),
        width = canvas.width,
        height = canvas.height,
        len = 4 * width * height,
        result = context.createImageData(width, height),
        offset = new Array(len),
        delta = new Array(len),
        timestamp = Date.now(),
        frames = 0,
        value = 0,
        ticker;

    function init() {
        var i, source, target;

        canvas.style.opacity = 0;

        context.drawImage(image1, 0, 0);
        source = context.getImageData(0, 0, width, height);
        context.drawImage(image2, 0, 0);
        target = context.getImageData(0, 0, width, height);

        result = context.createImageData(width, height);
        for (i = 0; i < len; i += 1) {
            offset[i] = target.data[i];
            delta[i] = source.data[i] - target.data[i];
            result.data[i] = 255;
        }

        context.fillStyle = '#fff';
        context.fillRect(0, 0, width, height);

        canvas.style.opacity = 1;
    }

    function tween(factor) {
        var i, r;
        r = result.data;
        for (i = 0; i < len; i += 4) {
            r[i] = offset[i] + delta[i] * factor;
            r[i + 1] = offset[i + 1] + delta[i + 1] * factor;
            r[i + 2] = offset[i + 2] + delta[i + 2] * factor;
        }
        context.putImageData(result, 0, 0);
        frames += 1;
    }

    function start() {
        value = 0;
        timestamp = Date.now();
        ticker = window.setInterval(function () {
            value += 0.1;
            tween(0.5 + 0.5 * Math.sin(value));
        }, 1000 / 60);
    }

    function stop() {
        window.clearInterval(ticker);
    }

    function frameRate() {
        return (1000 * frames / (Date.now() - timestamp));
    }

    if (typeof Int16Array !== 'undefined') {
        offset = new Int16Array(len);
        delta = new Int16Array(len);
    }

    init();

    return {
        value: value,
        start: start,
        stop: stop,
        frameRate: frameRate
    };
}
