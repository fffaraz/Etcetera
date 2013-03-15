/*
  This file is part of the Ofi Labs X2 project.

  Copyright (C) 2012 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

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

/*jslint plusplus:true browser:true*/
/*global Int16Array:true */

function Underwater(canvas, image, amplitude, frequency) {

    'use strict';

    var context = canvas.getContext('2d'),
        width = canvas.width,
        stride = width * 4,
        height = canvas.height,
        result = context.createImageData(width, height),
        pixels = new Array(4 * width * height),
        timestamp = Date.now(),
        interval = 1000 / 60,
        frames = 0,
        ticker;

    function init() {
        var i, source;

        context.drawImage(image, 0, 0);
        source = context.getImageData(0, 0, width, height);

        result = context.createImageData(width, height);
        for (i = 0; i < 4 * height * width; i += 1) {
            pixels[i] = source.data[i];
            result.data[i] = 255;
        }
        apply();
    }

    function apply() {
        var r, T, x, y, xs, ys, dest, src;

        r = result.data;

        T = frames * interval * frequency / 1000;
        for (x = amplitude; x < width - amplitude; ++x) {
            for (y = amplitude; y < height - amplitude; ++y) {
                xs = amplitude * Math.sin(2 * Math.PI * (3 * y / height + T));
                ys = amplitude * Math.cos(2 * Math.PI * (3 * x / width + T));
                xs = Math.round(xs);
                ys = Math.round(ys);
                dest = y * stride + x * 4;
                src = (y + ys) * stride + (x + xs) * 4;
                r[dest] = pixels[src];
                r[dest + 1] = pixels[src + 1];
                r[dest + 2] = pixels[src + 2];
            }
        }

        context.putImageData(result, 0, 0);
        frames += 1;

        // Force flushing the pending painting.
        context.getImageData(0, 0, 1, 1);
    }

    function start() {
        ticker = window.setInterval(apply, interval);
    }

    function frameRate() {
        return (1000 * frames / (Date.now() - timestamp));
    }

    if (typeof Int16Array !== 'undefined') {
        pixels = new Int16Array(4 * width * height);
    }

    init();

    return {
        start: start,
        frameRate: frameRate
    };
}
