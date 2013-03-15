/*
    This file is part of the OfiLabs X2 project.

    Copyright (C) 2010 Helder Correia <helder.pereira.correia@gmail.com>

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

var canvas = document.getElementById("plasmaeffect");
var context = canvas.getContext("2d");
var width = 640;
var height = 360;
var buffer = [];
var pattern = [];
var palette = new Array(256);
var timerInterval = 16;
var animationTimer = null;
var baseFunction = Math.sin;

var alpha = 20;
var alphaAdjust = 0.15;
var beta = 100;
var betaAdjust = 0.015;

var redComponent = 0;
var greenComponent = 255;
var blueComponent = 0;

var redComponentChangeFactor = 2;
var greenComponentChangeFactor = -2;
var blueComponentChangeFactor = 2;

var fps = 0;
var oldFps = 0;

function Rgb(r, g, b) {
    this.red = r;
    this.green = g;
    this.blue = b;
}

function setUp() {
    var _width = width, _height = height;
    var maxDimension = Math.max(_width, _height);
    var f = new Array(maxDimension);
    var _alpha = alpha, _alphaAdjust = alphaAdjust, _beta = beta, _betaAdjust = betaAdjust;
    var _baseFunction = baseFunction;
    var _cos = Math.cos, _round = Math.round;
    var x;

    for (x = 0; x < maxDimension; ++x)
        f[x] = _round(_alpha * _baseFunction(x * _alphaAdjust) + _beta * _cos(x * _betaAdjust));

    var _palette = palette;
    var _redComponentChangeFactor = redComponentChangeFactor,
        _greenComponentChangeFactor = greenComponentChangeFactor,
        _blueComponentChangeFactor = blueComponentChangeFactor;
    var r = redComponent, g = greenComponent, b = blueComponent;
    var _Rgb = Rgb, newRgb;

    for (x = 0; x < 128; ++x) {
        newRgb = new _Rgb(r, g, b);
        _palette[x] = _palette[255-x] = newRgb;
        r += _redComponentChangeFactor;
        g += _greenComponentChangeFactor;
        b += _blueComponentChangeFactor;
    }

    var _pattern = pattern;
    var y;
    var _abs = Math.abs;

    for (y = 0; y < _height; ++y)
        for (x = 0; x < _width; ++x)
            _pattern[y * _width + x] = _abs(f[x] + f[y]) % 255;
}

function setBaseFunction(f) {
    baseFunction = f;
    setUp();
}

function setBaseColor(c) {
    if (c == "red") {
        redComponent   = 255; redComponentChangeFactor   = -2;
        greenComponent = 0;   greenComponentChangeFactor = 2;
        blueComponent  = 0;   blueComponentChangeFactor  = 2;
    } else if (c == "green") {
        redComponent   = 0;   redComponentChangeFactor   = 2;
        greenComponent = 255; greenComponentChangeFactor = -2;
        blueComponent  = 0;   blueComponentChangeFactor  = 2;
    } else {
        redComponent   = 0;   redComponentChangeFactor   = 2;
        greenComponent = 0;   greenComponentChangeFactor = 2;
        blueComponent  = 255; blueComponentChangeFactor  = -2;
    }
    setUp();
}

function toggleAnimation() {
    if (animationTimer) {
        clearInterval(animationTimer);
        animationTimer = null;
    } else
        animationTimer = setInterval(paint, timerInterval);
}

function resize(mode) {
    if (animationTimer)
        toggleAnimation();

    if (mode == "grow") {
        width = Math.round(width * 1.2);
        height = Math.round(height * 1.2);
    } else if (mode == "shrink") {
        width = Math.round(width * 0.8);
        height = Math.round(height * 0.8);
    }

    canvas.width = width;
    canvas.height = height;
    pattern = new Array(width * height);
    context.fillStyle = "rgba(0, 0, 0, 255)";
    context.fillRect(0, 0, width, height);
    buffer = context.getImageData(0, 0, width, height);

    setUp();
    toggleAnimation();
}

function paint() {
    var _palette = palette;
    var i;

    for (i = 0; i < 255; ++i)
        _palette[i] = _palette[i+1];
    _palette[255] = _palette[0];

    var x, y, pal;
    var data = buffer.data;
    var _pattern = pattern;
    var _width = width, _height = height;

    for (y = 0; y < _height; ++y)
        for (x = 0; x < _width; ++x) {
            i = y * _width + x;
            pal = _palette[_pattern[i]];
            i <<= 2;
            data[i] = pal.red;
            data[i+1] = pal.green;
            data[i+2] = pal.blue;
        }

    context.putImageData(buffer, 0, 0);
    ++fps;
}

function updateFps() {
    document.getElementById("fps").innerText = (fps - oldFps);
    oldFps = fps;
}

function setParameters(a, aAdjust, b, bAdjust) {
    alpha = a; alphaAdjust = aAdjust;
    beta = b; betaAdjust = bAdjust;
    setUp();
}

function changeAnimation(msec) {
    if (msec <= 250 && msec >= 10) {
        if (animationTimer)
            clearInterval(animationTimer);
        timerInterval = msec;
        animationTimer = setInterval(paint, msec);
    }
}


function onKeyPress(event) {
    event = (event) ? event : window.event;
    var k = event.keyCode;
    switch (k) {
        case 116: /* 't' */ setBaseFunction(Math.tan); break;
        case 115: /* 's' */ setBaseFunction(Math.sin); break;
        case 114: /* 'r' */ setBaseColor("red"); break;
        case 103: /* 'g' */ setBaseColor("green"); break;
        case 98:  /* 'b' */ setBaseColor("blue"); break;

        case 104: /* 'h' */ toggleAnimation(); break;
        case 79:  /* 'o' */ changeAnimation(timerInterval + 5); break;
        case 80:  /* 'p' */ changeAnimation(timerInterval - 5); break;

        case 43:  /* '+' */ resize("grow"); break;
        case 45:  /* '-' */ resize("shrink"); break;

        case 48: /* '0' */ setParameters(20, 0.15, 100, 0.015);  break;
        case 49: /* '1' */ setParameters(0, -0.046, -4, 0.0086); break;
        case 50: /* '2' */ setParameters(-16, -0.012, 4, -0.0266); break;
        case 51: /* '3' */ setParameters(-58, -0.002, 4, -0.1146); break;
        case 52: /* '4' */ setParameters(-58, -0.002, -4, -0.0138); break;
        case 53: /* '5' */ setParameters(4, 0.102, 12, -0.0686); break;
        case 54: /* '6' */ setParameters(-2, 0.244, 1780, -0.0007); break;
        case 55: /* '7' */ setParameters(-34, 0.092, 1780, -0.0007); break;
        case 56: /* '8' */ setParameters(-130, 0.008, -12, -0.043); break;
        case 57: /* '9' */ setParameters(-38, 0.002, 12, -0.0662); break;

        case 37: // 'arrow left'
            if (event.shiftKey == true)
                betaAdjust -= 0.0008;
            else
                alphaAdjust -= 0.002;
            setUp();
            break;
        case 38: // 'arrow up'
            if (event.shiftKey == true)
                beta += 8;
            else
                alpha += 2;
            setUp();
            break;
        case 39: // 'arrow right'
            if (event.shiftKey == true)
                betaAdjust += 0.0008;
            else
                alphaAdjust += 0.002;
            setUp();
            break;
        case 40: // 'arrow down'
            if (event.shiftKey == true)
                beta -= 8;
            else
                alpha -= 2;
            setUp();
            break;
    }
}

function start() {
    document.onkeypress = onKeyPress;
    document.onkeydown= onKeyPress;
    setBaseColor('blue')
    resize();
    setInterval(updateFps, 1000);
}

start();
